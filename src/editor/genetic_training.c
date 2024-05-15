#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "../genetic_training.h"

#include "../debug.h"
#include "../editor.h"
#include "../scene.h"
#include "cimgui.h"
#include "cimgui_impl.h"
#include "cimplot.h"
#include "common.h"
#include <float.h>
#include <math.h>
#include <stdlib.h>

static int N_ENTITIES_WITHOUT_SCORER;
static int ENTITIES_WITHOUT_SCORERS[MAX_N_ENTITIES];

static Array SCORES[MAX_N_ENTITIES_TO_TRAIN];
static float MIN_SCORE = FLT_MAX;
static float MAX_SCORE = -FLT_MAX;

static void render_genetic_training_menu_bar(void) {
    if (igBeginMenu("Genetic Training", 1)) {
        if (igBeginMenu("File", 1)) {
            if (menu_item("Open (TODO: Not implemented)", "", false, 1)) {
            }
            if (menu_item("Save As (TODO: Not implemented)", "", false, 1)) {
            }
            igEndMenu();
        }

        igSeparator();
        if (menu_item("Quit", "Ctrl+Q", false, 1)) {
            EDITOR.is_editing_genetic_training = 0;
        }

        igEndMenu();
    }

    if (EDITOR.key.ctrl && EDITOR.key.q) {
        EDITOR.is_editing_genetic_training = 0;
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

        Scorer *scorer = &SCENE.scorers[entity];
        if (igButton("Fix", IG_VEC2_ZERO)) {
            reset_scorer(scorer);
            SCENE.components[entity] |= SCORER_COMPONENT;
        }
    }
}

static void render_entities_to_train(void) {
    int n_entities_to_train = GENETIC_TRAINING->progress.n_entities_to_train;
    igText("%d Entities to train:", n_entities_to_train);
    for (int e = 0; e < n_entities_to_train; ++e) {
        int entity = GENETIC_TRAINING->progress.entities_to_train[e];
        static char str[16];
        sprintf(str, "  Entity: %d", entity);

        if (igBeginMenu(str, 1)) {
            igCheckbox("is frozen", (bool *)&GENETIC_TRAINING->progress.is_frozen[e]);
            SimulationStatus status = GENETIC_TRAINING->progress.status;
            if (status == SIMULATION_NOT_STARTED) {
                Scorer *scorer = &SCENE.scorers[entity];
                render_scorer_weights_inspector(scorer);
            } else if (GENETIC_TRAINING->progress.episode > 0) {
                Scorer *scorer = &GENETIC_TRAINING->progress.best_scorers[e];
                igText("Total: %.2f", get_total_score(scorer));
                igSeparator();
                render_scorer_values_inspector(scorer);
            }
            igEndMenu();
        }
    }
}

static void render_genetic_training_parameters(void) {
    int is_started = GENETIC_TRAINING->progress.status == SIMULATION_NOT_STARTED;
    if (is_started) {
        igText("Simulation:");
        ig_drag_float(
            "Timestep (ms)", &GENETIC_TRAINING->simulation.dt_ms, 1.0, 100.0, 1.0, 0
        );
        igSeparator();
    }

    igText("Population:");
    ig_drag_float(
        "Episode time (s)", &GENETIC_TRAINING->population.episode_time, 5.0, 600.0, 1.0, 0
    );

    if (is_started) {
        ig_drag_int(
            "Episodes", &GENETIC_TRAINING->population.n_episodes, 10, MAX_N_EPISODES, 1, 0
        );
    }

    igText("Evolution:");
    ig_drag_int("Elite streak", &GENETIC_TRAINING->evolution.elite_streak, 1, 100, 1, 0);
    ig_drag_float(
        "Elite ratio", &GENETIC_TRAINING->evolution.elite_ratio, 0.01, 0.9, 0.01, 0
    );
    ig_drag_float(
        "Mutation rate", &GENETIC_TRAINING->evolution.mutation_rate, 0.01, 0.9, 0.01, 0
    );
    ig_drag_float(
        "Mutation strength",
        &GENETIC_TRAINING->evolution.mutation_strength,
        0.01,
        0.9,
        0.01,
        0
    );
}

static void render_genetic_training_controls(void) {
    SimulationStatus *status = &GENETIC_TRAINING->progress.status;
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
            int can_start = 1;

            if (GENETIC_TRAINING->progress.n_entities_to_train
                > MAX_N_ENTITIES_TO_TRAIN) {
                igTextColored(
                    IG_RED_COLOR,
                    "ERROR: Can't start training, freeze some brains "
                    "(MAX_N_ENTITIES_TO_TRAIN: %d)",
                    MAX_N_ENTITIES_TO_TRAIN
                );
                can_start = 0;
            }

            if (GENETIC_TRAINING->population.n_episodes > MAX_N_EPISODES) {
                igTextColored(
                    IG_RED_COLOR,
                    "ERROR: Can't start training, reduce "
                    "population.n_episodes "
                    "(MAX_N_EPISODES: %d)",
                    MAX_N_EPISODES
                );
                can_start = 0;
            }

            if (can_start && igButton("Start", IG_VEC2_ZERO)) {
                start_genetic_training();
            }
            break;
        }
    }

    if (GENETIC_TRAINING_PID != 0 && GENETIC_TRAINING_PID != -1) {
        ig_same_line();
        if (igButton("Stop", IG_VEC2_ZERO)) {
            for (int e = 0; e < GENETIC_TRAINING->progress.n_entities_to_train; ++e) {
                destroy_array(&SCORES[e]);
            }
            kill_genetic_training();
            reset_genetic_training();
        }
    }
}

static void render_evolution_progress_bar(void) {
    GeneticTraining *params = GENETIC_TRAINING;
    float fraction = (float)params->progress.episode / params->population.n_episodes;
    ImVec2 size_arg = {0.0, 0.0};
    igProgressBar(fraction, size_arg, "");
    ig_same_line();
    igText("Generation: %d", params->progress.generation);

    fraction = (float)params->progress.episode_time / params->population.episode_time;
    igProgressBar(fraction, size_arg, "");
    ig_same_line();
    igText("Episode: %d/%d", params->progress.episode, params->population.n_episodes);
}

// TODO: Bug! Plots are drawn for entities which are present in the
// main process. E.g if the entity in main process dies, it disappears
// from the genetic training process (or at lease genetic training plot)
static void render_evolution_plots(void) {
    ImPlotContext *ctx = ImPlot_CreateContext();
    ImPlot_SetCurrentContext(ctx);

    float n = GENETIC_TRAINING->progress.generation + 1;
    int n_entities_to_train = GENETIC_TRAINING->progress.n_entities_to_train;

    if (ImPlot_BeginPlot("Best scores", IG_VEC2_ZERO, 0)) {
        int offset = 0;
        int stride = sizeof(float);

        ImPlot_SetupAxis(ImAxis_X1, "Generation", 0);
        ImPlot_SetupAxis(ImAxis_Y1, "Score", 0);
        ImPlot_SetupAxisLimits(ImAxis_X1, 0.0, (int)(1.05 * n), 0);
        ImPlot_SetupAxisLimits(
            ImAxis_Y1,
            MIN_SCORE - fabs(0.05 * MIN_SCORE),
            MAX_SCORE + fabs(0.05 * MAX_SCORE),
            0
        );
        for (int e = 0; e < n_entities_to_train; ++e) {
            float *ys = SCORES[e].data;
            char str[16];
            sprintf(str, "Entity: %d", GENETIC_TRAINING->progress.entities_to_train[e]);
            ImPlot_PlotLine_FloatPtrInt(str, ys, n, 1.0, 0.0, 0, 0, stride);
        }

        ImPlot_EndPlot();
    }

    if (ImPlot_BeginPlot("Elite streaks", IG_VEC2_ZERO, 0)) {
        ImPlot_SetupAxes("Streak", "N Episodes", 0, 0);
        ImPlot_SetupAxisLimits(ImAxis_X1, 1, 20, 0);
        ImPlot_SetupAxisLimits(ImAxis_Y1, 0, 5, 0);
        ImPlotRange range = {1, 20};

        float bar_scale = 1.0;
        int episode = GENETIC_TRAINING->progress.episode;
        int hist_flags = 0;
        for (int e = 0; e < n_entities_to_train; ++e) {
            if (GENETIC_TRAINING->progress.is_frozen[e] == 1) {
                continue;
            }

            float *streaks = GENETIC_TRAINING->progress.elite_streaks[e];
            char str[16];
            sprintf(str, "Entity: %d", GENETIC_TRAINING->progress.entities_to_train[e]);
            ImPlot_PlotHistogram_FloatPtr(
                str, streaks, episode, 40, bar_scale, range, hist_flags
            );
        }

        ImPlot_EndPlot();
    }

    ImPlot_DestroyContext(ctx);
}

static void update_counters(void) {
    N_ENTITIES_WITHOUT_SCORER = 0;
    GENETIC_TRAINING->progress.n_entities_to_train = 0;
    for (int entity = 0; entity < SCENE.n_entities; ++entity) {
        if (!check_if_entity_has_component(entity, CONTROLLER_COMPONENT)) {
            continue;
        }

        Controller controller = SCENE.controllers[entity];
        ControllerType type = controller.type;
        if (type != BRAIN_AI_CONTROLLER) {
            continue;
        }

        char *key = controller.c.brain_ai.key;
        if (key[0] == '\0') {
            continue;
        }
        Brain *brain = get_or_load_brain(key);

        int has_scorer = check_if_entity_has_component(entity, SCORER_COMPONENT);
        if (brain->params.is_trainable) {
            if (has_scorer) {
                GENETIC_TRAINING->progress
                    .entities_to_train[GENETIC_TRAINING->progress.n_entities_to_train++]
                    = entity;
            } else {
                ENTITIES_WITHOUT_SCORERS[N_ENTITIES_WITHOUT_SCORER++] = entity;
            }
        }
    }
}

static void update_evolution_history(void) {
    GeneticTraining *params = GENETIC_TRAINING;

    if (SCORES[0].data == NULL) {
        for (int e = 0; e < MAX_N_ENTITIES_TO_TRAIN; ++e) {
            SCORES[e] = init_array();
        }
    }

    int gen = params->progress.generation;
    for (int e = 0; e < params->progress.n_entities_to_train; ++e) {
        Scorer *scorer = &params->progress.best_scorers[e];
        float val = get_total_score(scorer);

        int n_scores = SCORES[e].length;
        if (n_scores == gen) {
            if (n_scores > 0) {
                float prev_val = array_get(&SCORES[e], n_scores - 1);
                MIN_SCORE = min(MIN_SCORE, prev_val);
                MAX_SCORE = max(MAX_SCORE, prev_val);
            }
            array_push(&SCORES[e], val);
        } else if (n_scores == gen + 1) {
            SCORES[e].data[n_scores - 1] = val;
        } else if (n_scores > gen + 1) {
            fprintf(
                stderr,
                "ERROR: Number of scores (%d) is greater than the number "
                "of passed generations (%d). This is a bug\n",
                n_scores,
                gen + 1
            );
            exit(1);
        } else if (n_scores < gen) {
            fprintf(
                stderr,
                "ERROR: Number of scores (%d) is less than the number of "
                "previously passed generations (%d). This is a bug\n",
                n_scores,
                gen
            );
            exit(1);
        } else {
            fprintf(stderr, "ERROR: Unreachable. This is a bug\n");
            exit(1);
        }
    }
}

static void render_genetic_training_progress(void) {
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

    // TODO: Add possibility to selecte a specific scene to perform
    // genetic training on (like in profiler)

    if (GENETIC_TRAINING->progress.status == SIMULATION_NOT_STARTED) {
        igCheckbox("All immortal", (bool *)&DEBUG.gameplay.all_immortal);
    }

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
