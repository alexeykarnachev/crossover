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
static float MAX_SCORE = -FLT_MAX;
static float MIN_SCORE = FLT_MAX;

static Array SCORES[MAX_N_ENTITIES_TO_TRAIN] = {0};
static Array GENERATIONS = {0};

void init_genetic_training(GeneticTraining* genetic_training) {
    memset(genetic_training, 0, sizeof(GeneticTraining));
    genetic_training->simulation.dt_ms = 17.0;
    genetic_training->progress.status = SIMULATION_NOT_STARTED;
    for (int e = 0; e < MAX_N_ENTITIES_TO_TRAIN; ++e) {
        genetic_training->progress.scores[e] = -FLT_MAX;
    }
    genetic_training->population.live_time = 10.0;
    genetic_training->population.size = 10;
    genetic_training->evolution.elite_ratio = 0.1;
    genetic_training->evolution.mutation_rate = 0.01;
}

void reset_genetic_training(GeneticTraining* genetic_training) {
    if (genetic_training != NULL) {
        genetic_training->progress.status = SIMULATION_NOT_STARTED;
        genetic_training->progress.generation = 0;
        genetic_training->progress.individual = 0;
        genetic_training->progress.live_time = 0;
        memset(
            genetic_training->progress.scores,
            0,
            sizeof(genetic_training->progress.scores)
        );
    }

    N_BRAINS_TO_TRAIN = 0;
    N_ENTITIES_WITHOUT_SCORER = 0;
    N_ENTITIES_TO_TRAIN = 0;
    MAX_SCORE = -FLT_MAX;
    MIN_SCORE = FLT_MAX;

    for (int e = 0; e < MAX_N_ENTITIES_TO_TRAIN; ++e) {
        if (SCORES[e].data != NULL) {
            destroy_array(&SCORES[e]);
        }
    }

    if (GENERATIONS.data != NULL) {
        destroy_array(&GENERATIONS);
    }
}

static void update_evolution_history(void) {
    GeneticTraining* params = GENETIC_TRAINING;

    if (SCORES[0].data == NULL) {
        for (int e = 0; e < MAX_N_ENTITIES_TO_TRAIN; ++e) {
            SCORES[e] = init_array();
        }
        GENERATIONS = init_array();
    }

    int gen = params->progress.generation;
    if (GENERATIONS.length == 0 || array_peek(&GENERATIONS) < gen) {
        array_push(&GENERATIONS, gen);
    }

    for (int e = 0; e < N_ENTITIES_TO_TRAIN; ++e) {
        float val = params->progress.scores[e];

        MAX_SCORE = max(MAX_SCORE, val);
        MIN_SCORE = MIN_SCORE == -FLT_MAX ? val : min(MIN_SCORE, val);

        if (SCORES[e].length <= gen) {
            array_push(&SCORES[e], val);
        } else if (SCORES[e].length == gen + 1) {
            SCORES[e].data[SCORES[e].length - 1] = val;
        } else {
            fprintf(
                stderr,
                "ERROR: Generations are in descending order. This is a "
                "bug\n"
            );
            exit(1);
        }
    }
}

static void start_genetic_training(void) {
    pid_t pid;
    pid = fork();
    if (pid == -1) {
        perror("ERROR: Can't start Genetic Training\n");
    } else if (pid == 0) {
        GeneticTraining* params = GENETIC_TRAINING;
        SimulationStatus* status = &params->progress.status;
        *status = SIMULATION_RUNNING;

        for (int e = 0; e < N_ENTITIES_TO_TRAIN; ++e) {
            SCENE.scorers[ENTITIES_TO_TRAIN[e]].value = 0.0;
        }

        int generation = 0;
        while (*status == SIMULATION_RUNNING) {
            int individual = 0;

            float* scores = params->progress.scores;
            while (individual < params->population.size) {
                params->progress.live_time = 0.0;
                while (params->progress.live_time
                       < params->population.live_time) {
                    float dt = params->simulation.dt_ms / 1000.0;
                    update_scene(dt, 1);
                    params->progress.live_time += dt;

                    while (*status == SIMULATION_PAUSED) {
                        sleep(0.1);
                    }

                    if (*status == SIMULATION_NOT_STARTED) {
                        exit(0);
                    }
                }

                for (int e = 0; e < N_ENTITIES_TO_TRAIN; ++e) {
                    int entity = ENTITIES_TO_TRAIN[e];
                    Scorer* scorer = &SCENE.scorers[entity];
                    float score = scorer->value;
                    scores[e] = max(scores[e], score);

                    // TODO: Remove after testing:
                    // scores[e] = ((float)rand() / RAND_MAX) * 2.0 - 1.0;

                    scorer->value = 0.0;
                }

                params->progress.individual = ++individual;
                // TODO: Reset scene here!
            }

            params->progress.generation = ++generation;
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
            Brain* brain = &asset->a.brain;
            if (brain->params.is_trainable) {
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
            SimulationStatus status = GENETIC_TRAINING->progress.status;
            if (status == SIMULATION_NOT_STARTED) {
                render_scorer_weights_inspector(scorer);
            } else {
                igTextColored(
                    IG_YELLOW_COLOR,
                    "Can't change scorer weights if simulation is started"
                );
            }
            igEndMenu();
        }
    }
}

static void render_genetic_training_parameters(void) {
    igText("Simulation:");
    ig_drag_float(
        "Timestep (ms)",
        &GENETIC_TRAINING->simulation.dt_ms,
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
    SimulationStatus* status = &GENETIC_TRAINING->progress.status;
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

static void render_evolution_progress_bar(void) {
    GeneticTraining* params = GENETIC_TRAINING;
    float fraction = (float)params->progress.individual
                     / params->population.size;
    ImVec2 size_arg = {0.0, 0.0};
    igProgressBar(fraction, size_arg, "");
    ig_same_line();
    igText("Generation: %d", params->progress.generation);

    fraction = (float)params->progress.live_time
               / params->population.live_time;
    igProgressBar(fraction, size_arg, "");
    ig_same_line();
    igText(
        "Individual: %d/%d",
        params->progress.individual,
        params->population.size
    );
}

static void render_evolution_plots(void) {
    ImPlotContext* ctx = ImPlot_CreateContext();
    ImPlot_SetCurrentContext(ctx);

    if (ImPlot_BeginPlot("Evolution", IG_VEC2_ZERO, 0)) {
        int offset = 0;
        int stride = sizeof(float);
        int n = GENERATIONS.length;
        float* xs = GENERATIONS.data;

        ImPlot_SetupAxis(ImAxis_X1, "Generation", 0);
        ImPlot_SetupAxis(ImAxis_Y1, "Score", 0);
        ImPlot_SetupAxisLimits(ImAxis_X1, 0.0, (int)(1.05 * n), 0);
        ImPlot_SetupAxisLimits(
            ImAxis_Y1,
            MIN_SCORE - fabs(0.05 * MIN_SCORE),
            MAX_SCORE + fabs(0.05 * MAX_SCORE),
            0
        );
        for (int e = 0; e < N_ENTITIES_TO_TRAIN; ++e) {
            float* ys = SCORES[e].data;
            char str[16];
            sprintf(str, "Entity: %d", ENTITIES_TO_TRAIN[e]);
            ImPlot_PlotLine_FloatPtrFloatPtr(
                str, xs, ys, n, 0, offset, stride
            );
        }

        ImPlot_EndPlot();
    }
}

void render_genetic_training_progress(void) {
    SimulationStatus status = GENETIC_TRAINING->progress.status;

    igText("Progress:");
    if (status == SIMULATION_NOT_STARTED) {
        igTextColored(IG_YELLOW_COLOR, "Simulation not started");
    } else {
        update_evolution_history();
        render_evolution_progress_bar();
        render_evolution_plots();
    }
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
