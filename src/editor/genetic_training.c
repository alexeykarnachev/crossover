#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "../array.h"
#include "../editor.h"
#include "../scene.h"
#include "cimgui.h"
#include "cimgui_impl.h"
#include "cimplot.h"
#include "common.h"
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static char* BRAINS_TO_TRAIN_FILE_PATHS[MAX_N_ASSETS];
static int ENTITIES_WITHOUT_SCORERS[MAX_N_ENTITIES];
static int ENTITIES_TO_TRAIN[MAX_N_ENTITIES];
static int N_BRAINS_TO_TRAIN = 0;
static int N_ENTITIES_WITHOUT_SCORER = 0;
static int N_ENTITIES_TO_TRAIN = 0;

static void start_genetic_training(void) {
    pid_t pid;
    pid = fork();
    if (pid == -1) {
        perror("ERROR: Can't start Genetic Training\n");
    } else if (pid == 0) {
        GeneticTraining* params = GENETIC_TRAINING;
        SimulationStatus* status = &params->simulation.status;
        *status = SIMULATION_RUNNING;

        int generation = 0;
        while (*status == SIMULATION_RUNNING) {
            int individual = 0;

            static float scores[MAX_N_ENTITIES_TO_TRAIN] = {-FLT_MAX};
            while (individual < params->population.size) {
                int live_time = 0;
                while (live_time < params->population.live_time) {
                    update_scene(params->simulation.dt, 1);
                    live_time += params->simulation.dt;

                    while (*status == SIMULATION_PAUSED) {
                        sleep(0.1);
                    }
                }

                for (int e = 0; e < N_ENTITIES_TO_TRAIN; ++e) {
                    int entity = ENTITIES_TO_TRAIN[e];
                    Scorer* scorer = &SCENE.scorers[entity];
                    float score = scorer->value;
                    scores[e] = max(scores[e], score);

                    // TODO: Remove after tests
                    scores[e] = ((float)rand() / RAND_MAX);

                    scorer->value = 0.0;
                }

                params->progress.individual = individual++;
            }

            memcpy(params->progress.scores, scores, sizeof(scores));
            params->progress.generation = generation++;
        }
        exit(0);
    }
}

static void render_genetic_training_menu_bar(void) {
    if (igBeginMenu("Genetic Training Parameters", 1)) {
        if (igBeginMenu("File", 1)) {
            if (menu_item("Open (TODO: Not implemented)", "", false, 1)) {}
            if (menu_item(
                    "Save As (TODO: Not implemented)", "", false, 1
                )) {}
            igEndMenu();
        }

        igSeparator();
        if (menu_item("Reset (TODO: Not implemented)", "", false, 1)) {}

        if (menu_item("Quit", "Ctrl+Q", false, 1)) {
            EDITOR.is_editing_genetic_training = 0;
        }

        igEndMenu();
    }

    if (EDITOR.key.ctrl && EDITOR.key.q) {
        EDITOR.is_editing_genetic_training = 0;
    }
}

static char* get_brain_file_path(int entity) {
    int has_controller = check_if_entity_has_component(
        entity, CONTROLLER_COMPONENT
    );
    if (!has_controller) {
        return NULL;
    }

    Controller* controller = &SCENE.controllers[entity];
    ControllerType type = controller->type;
    if (type != BRAIN_AI_CONTROLLER) {
        return NULL;
    }

    return controller->c.brain_ai.brain_file_path;
}

static void update_counters(void) {
    static char* trainable_brain_file_paths[MAX_N_ASSETS];
    int n_trainable_brains = 0;

    for (int i = 0; i < MAX_N_ASSETS; ++i) {
        Asset* asset = &ASSETS[i];
        if (asset->type == BRAIN_ASSET) {
            Brain brain = asset->a.brain;
            if (brain.params.is_trainable) {
                trainable_brain_file_paths[n_trainable_brains++]
                    = asset->file_path;
            }
        }
    }

    N_BRAINS_TO_TRAIN = 0;
    N_ENTITIES_WITHOUT_SCORER = 0;
    N_ENTITIES_TO_TRAIN = 0;
    for (int entity = 0; entity < SCENE.n_entities; ++entity) {
        char* file_path = get_brain_file_path(entity);
        int has_scorer = check_if_entity_has_component(
            entity, SCORER_COMPONENT
        );
        if (file_path == NULL) {
            continue;
        }

        int has_trainable_brain = 0;
        for (int i = 0; i < n_trainable_brains; ++i) {
            char* fp = trainable_brain_file_paths[i];
            if (fp == NULL || strcmp(file_path, fp) == 0) {
                has_trainable_brain = 1;
                if (fp != NULL) {
                    BRAINS_TO_TRAIN_FILE_PATHS[N_BRAINS_TO_TRAIN++] = fp;
                    trainable_brain_file_paths[i] = NULL;
                }
                break;
            }
        }

        if (has_trainable_brain) {
            if (has_scorer) {
                ENTITIES_TO_TRAIN[N_ENTITIES_TO_TRAIN++] = entity;
            } else {
                ENTITIES_WITHOUT_SCORERS[N_ENTITIES_WITHOUT_SCORER++]
                    = entity;
            }
        }
    }
}

static void render_brains_to_train(void) {
    igText("%d Brains to train:", N_BRAINS_TO_TRAIN);
    for (int i = 0; i < N_BRAINS_TO_TRAIN; ++i) {
        char* name = get_short_file_path(BRAINS_TO_TRAIN_FILE_PATHS[i]);
        igText("  %s", name);
    }
}

static void render_entities_without_scorer(void) {
    if (N_ENTITIES_WITHOUT_SCORER > 0) {
        igTextColored(IG_RED_COLOR, "%d", N_ENTITIES_WITHOUT_SCORER);
    } else {
        igTextColored(IG_GREEN_COLOR, "%d", 0);
    }
    ig_same_line();
    igText("missed Scorers:");
    for (int i = 0; i < N_ENTITIES_WITHOUT_SCORER; ++i) {
        int entity = ENTITIES_WITHOUT_SCORERS[i];
        static char str[16];
        sprintf(str, "Entity: %d", entity);

        Scorer* scorer = &SCENE.scorers[entity];
        ig_same_line();
        if (igButton("Fix", IG_VEC2_ZERO)) {
            reset_scorer(scorer);
            SCENE.components[entity] |= SCORER_COMPONENT;
        }
    }
}

static void render_entities_to_train(void) {
    igText("%d Entities to train:", N_ENTITIES_TO_TRAIN);
    for (int i = 0; i < N_ENTITIES_TO_TRAIN; ++i) {
        int entity = ENTITIES_TO_TRAIN[i];
        static char str[16];
        sprintf(str, "  Entity: %d", entity);

        Scorer* scorer = &SCENE.scorers[entity];
        if (igBeginMenu(str, 1)) {
            render_scorer_weights_inspector(scorer);
            igEndMenu();
        }
    }
}

static void render_genetic_training_parameters(void) {
    igText("Simulation:");
    ig_drag_float(
        "Timestep (ms)",
        &GENETIC_TRAINING->simulation.dt,
        1.0,
        100.0,
        1.0,
        0
    );
    igSeparator();

    igText("Population:");
    ig_drag_float(
        "Live time (s)",
        &GENETIC_TRAINING->population.live_time,
        5.0,
        600.0,
        1.0,
        0
    );
    ig_drag_int(
        "Population size",
        &GENETIC_TRAINING->population.size,
        10,
        MAX_POPULATION_SIZE,
        1,
        0
    );

    igText("Evolution:");
    ig_drag_float(
        "Elite (ratio)",
        &GENETIC_TRAINING->evolution.elite_ratio,
        0.01,
        0.9,
        0.01,
        0
    );
    ig_drag_float(
        "Mutation (rate)",
        &GENETIC_TRAINING->evolution.mutation_rate,
        0.01,
        0.9,
        0.01,
        0
    );
}

void render_genetic_training_controls(void) {
    SimulationStatus* status = &GENETIC_TRAINING->simulation.status;
    switch (*status) {
        case SIMULATION_RUNNING: {
            if (igButton("Pause", IG_VEC2_ZERO)) {
                *status = SIMULATION_PAUSED;
            }
            break;
        }
        case SIMULATION_PAUSED: {
            if (igButton("Continue", IG_VEC2_ZERO)) {
                *status = SIMULATION_RUNNING;
            }
            break;
        }
        case SIMULATION_NOT_STARTED: {
            char* str[128];
            int can_start = 1;
            if (N_BRAINS_TO_TRAIN > MAX_N_BRAINS_TO_TRAIN) {
                igTextColored(
                    IG_RED_COLOR,
                    "ERROR: Can't start training, freeze some brains "
                    "(MAX_N_BRAINS_TO_TRAIN: %d)",
                    MAX_N_BRAINS_TO_TRAIN
                );
                can_start = 0;
            }

            if (N_ENTITIES_TO_TRAIN > MAX_N_ENTITIES_TO_TRAIN) {
                igTextColored(
                    IG_RED_COLOR,
                    "ERROR: Can't start training, freeze some brains "
                    "(MAX_N_ENTITIES_TO_TRAIN: %d)",
                    MAX_N_ENTITIES_TO_TRAIN
                );
                can_start = 0;
            }

            if (GENETIC_TRAINING->population.size > MAX_POPULATION_SIZE) {
                igTextColored(
                    IG_RED_COLOR,
                    "ERROR: Can't start training, reduce population size "
                    "(MAX_POPULATION_SIZE: %d)",
                    MAX_POPULATION_SIZE
                );
                can_start = 0;
            }

            if (can_start && igButton("Start", IG_VEC2_ZERO)) {
                start_genetic_training();
            }
            break;
        }
    }
}

void render_genetic_training_progress(void) {
    static Array scores = {0};
    static Array generations = {0};
    static float max_score = -FLT_MAX;
    static float min_score = FLT_MAX;
    if (scores.data == NULL) {
        scores = init_array();
        generations = init_array();
    }

    igText("Progress:");

    GeneticTraining* params = GENETIC_TRAINING;
    float fraction = (float)params->progress.individual
                     / params->population.size;
    ImVec2 size_arg = {0.0, 0.0};
    igProgressBar(fraction, size_arg, "");
    ig_same_line();
    igText("Generation: %d", params->progress.generation);

    ImPlotContext* ctx = ImPlot_CreateContext();
    ImPlot_SetCurrentContext(ctx);

    if (ImPlot_BeginPlot("Evolution", IG_VEC2_ZERO, 0)) {
        int gen = params->progress.generation;
        float val = params->progress.scores[0];
        if (generations.length == 0 || array_peek(&generations) < gen) {
            array_push(&generations, gen);
            array_push(&scores, val);
            max_score = max(max_score, val);
            min_score = min(min_score, val);
        } else if (array_peek(&generations) == gen) {
            scores.data[scores.length - 1] = val;
        } else {
            fprintf(
                stderr,
                "ERROR: Generations are in descending order. This is a "
                "bug\n"
            );
            exit(1);
        }

        int offset = 0;
        int stride = sizeof(float);
        float* xs = generations.data;
        float* ys = scores.data;
        int n = scores.length;

        ImPlot_SetupAxis(ImAxis_X1, "Generation", 0);
        ImPlot_SetupAxis(ImAxis_Y1, "Score", 0);
        ImPlot_SetupAxisLimits(ImAxis_X1, 0.0, (int)(1.05 * n), 0);
        ImPlot_SetupAxisLimits(
            ImAxis_Y1,
            min_score - fabs(0.05 * min_score),
            max_score + fabs(0.05 * max_score),
            0
        );

        ImPlot_PushStyleColor_Vec4(ImPlotCol_Line, IG_RED_COLOR);
        ImPlot_PlotLine_FloatPtrFloatPtr(
            "Score", xs, ys, n, 0, offset, stride
        );
        ImPlot_PopStyleColor(1);

        ImPlot_EndPlot();
    }
    // ImGui::SameLine();
    // ImPlot::ColormapScale(hist_flags & ImPlotHistogramFlags_Density ?
    // "Density" : "Count",0,max_count,ImVec2(100,0));
    // ImPlot_PopColormap(1);
}

void render_genetic_training_editor(void) {
    update_counters();

    render_genetic_training_menu_bar();
    igSeparator();

    render_brains_to_train();
    igSeparator();

    render_entities_without_scorer();
    igSeparator();

    render_entities_to_train();
    igSeparator();

    render_genetic_training_parameters();
    igSeparator();

    render_genetic_training_progress();
    igSeparator();

    render_genetic_training_controls();
}
