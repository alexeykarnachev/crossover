#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "../array.h"
#include "../debug.h"
#include "../editor.h"
#include "../scene.h"
#include "../utils.h"
#include "cimgui.h"
#include "cimgui_impl.h"
#include "cimplot.h"
#include "common.h"
#include "signal.h"
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

pid_t GENETIC_TRAINING_PID = -1;

static char* BRAINS_TO_TRAIN_FILE_PATHS[MAX_N_BRAINS];
static int ENTITIES_WITHOUT_SCORERS[MAX_N_ENTITIES];
static int ENTITIES_TO_TRAIN[MAX_N_ENTITIES];
static int N_ENTITIES_WITHOUT_SCORER;
static int N_ENTITIES_TO_TRAIN;

static Array SCORES[MAX_N_ENTITIES_TO_TRAIN];
static Array GENERATIONS;

static float BRAIN_ELITE_STREAKS[MAX_N_ENTITIES_TO_TRAIN][MAX_N_EPISODES];
static Brain GENERATION_ELITE_BRAINS[MAX_N_ENTITIES_TO_TRAIN]
                                    [MAX_N_EPISODES];
static Brain GENERATION_BRAINS[MAX_N_ENTITIES_TO_TRAIN][MAX_N_EPISODES];

static void reset_genetic_training(void) {
    GeneticTraining* training = GENETIC_TRAINING;

    memset(&training->progress, 0, sizeof(training->progress));

    // TODO: Move these array to the GeneticTraining structure
    for (int e = 0; e < N_ENTITIES_TO_TRAIN; ++e) {
        SCORES[e].length = 0;
    }
    GENERATIONS.length = 0;

    training->simulation.dt_ms = 17.0;

    training->population.episode_time = 60.0;
    training->population.n_episodes = 100;

    training->evolution.elite_streak = 5;
    training->evolution.elite_ratio = 0.15;
    training->evolution.mutation_rate = 0.3;
    training->evolution.mutation_strength = 0.15;
}

static int GENETIC_TRAINING_INITIALIZED = 0;
void init_genetic_training(void) {
    if (GENETIC_TRAINING_INITIALIZED == 1) {
        fprintf(
            stderr,
            "ERROR: GENETIC_TRAINING could be initialized only once\n"
        );
        exit(1);
    }
    memset(GENETIC_TRAINING, 0, sizeof(GeneticTraining));
    reset_genetic_training();

    GENETIC_TRAINING_INITIALIZED = 1;
    printf("DEBUG: GENETIC_TRAINING initialized\n");
}

static int GENETIC_TRAINING_DESTROYED = 0;
void destroy_genetic_training(void) {
    if (GENETIC_TRAINING_DESTROYED == 1) {
        fprintf(
            stderr,
            "ERROR: GENETIC_TRAINING could be destroyed only once\n"
        );
        exit(1);
    }

    if (GENETIC_TRAINING_INITIALIZED == 0) {
        fprintf(
            stderr, "ERROR: Can't destroy uninitialized GENETIC_TRAINING\n"
        );
        exit(1);
    }

    GeneticTraining* training = GENETIC_TRAINING;
    if (training != NULL) {
        training->progress.status = SIMULATION_NOT_STARTED;
        training->progress.generation = 0;
        training->progress.episode = 0;
        training->progress.episode_time = 0;

        printf("DEBUG: GENETIC_TRAINING destroyed\n");
    }

    N_ENTITIES_WITHOUT_SCORER = 0;
    N_ENTITIES_TO_TRAIN = 0;
    // TODO: Move these array to the GeneticTraining structure
    for (int e = 0; e < MAX_N_ENTITIES_TO_TRAIN; ++e) {
        if (SCORES[e].data != NULL) {
            destroy_array(&SCORES[e]);
        }
    }

    if (GENERATIONS.data != NULL) {
        destroy_array(&GENERATIONS);
    }

    GENETIC_TRAINING_DESTROYED = 1;
}

void kill_genetic_training(void) {
    if (GENETIC_TRAINING_PID > 0) {
        GENETIC_TRAINING->progress.status = SIMULATION_NOT_STARTED;
        kill(GENETIC_TRAINING_PID, SIGTERM);
        GENETIC_TRAINING_PID = -1;

        printf("DEBUG: GENETIC_TRAINING killed\n");
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
        Scorer* scorer = &params->progress.best_scorers[e];
        float val = get_total_score(scorer);

        int n_scores = SCORES[e].length;
        if (n_scores == gen) {
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

static void start_genetic_training(void) {
    GENETIC_TRAINING_PID = fork();
    if (GENETIC_TRAINING_PID == -1) {
        perror("ERROR: Can't start Genetic Training\n");
    } else if (GENETIC_TRAINING_PID == 0) {
        ResultMessage res_msg = {0};
        save_scene(".tmp.xscene", &res_msg);

        GeneticTraining* params = GENETIC_TRAINING;
        SimulationStatus* status = &params->progress.status;
        *status = SIMULATION_RUNNING;

        for (int e = 0; e < N_ENTITIES_TO_TRAIN; ++e) {
            int entity = ENTITIES_TO_TRAIN[e];
            BrainAIController* ai = &SCENE.controllers[entity].c.brain_ai;

            static char new_key[MAX_PATH_LENGTH + 128] = {0};
            // TODO: Increment file version is not enough.
            // The entity name also needs to be a part of the name.
            // Otherwise entities with the same starting brain will
            // overwrite each other
            char* new_key_p = increment_file_name_version(ai->key);
            if (strlen(new_key_p) > sizeof(new_key) - 1) {
                fprintf(
                    stderr,
                    "ERROR: New brain file name is longer than allowed "
                    "max length\n"
                );
                free(new_key_p);
                exit(1);
            }
            strcpy(new_key, new_key_p);
            free(new_key_p);

            for (int i = 0; i < params->population.n_episodes; ++i) {
                Brain* dst_brain = &GENERATION_BRAINS[e][i];
                clone_key_brain_into(dst_brain, ai->key, i != 0);
                strcpy(dst_brain->params.key, new_key);
            }
        }

        int generation = 0;
        while (*status == SIMULATION_RUNNING) {
            int episode = 0;

            ResultMessage res_msg = {0};
            Scorer* best_scorers = params->progress.best_scorers;
            while (episode < params->population.n_episodes) {
                for (int e = 0; e < N_ENTITIES_TO_TRAIN; ++e) {
                    reset_scorer(&SCENE.scorers[ENTITIES_TO_TRAIN[e]]);
                    int entity = ENTITIES_TO_TRAIN[e];
                    BrainAIController* ai
                        = &SCENE.controllers[entity].c.brain_ai;
                    Brain* brain = &GENERATION_BRAINS[e][episode];
                    add_brain_clone(brain, 1);
                    strcpy(ai->key, brain->params.key);
                }

                params->progress.episode_time = 0.0;
                while (params->progress.episode_time
                       < params->population.episode_time) {
                    float dt = params->simulation.dt_ms / 1000.0;
                    update_scene(dt, 1);
                    params->progress.episode_time += dt;

                    while (*status == SIMULATION_PAUSED) {
                        sleep(0.1);
                    }

                    if (*status == SIMULATION_NOT_STARTED) {
                        exit(0);
                    }
                }

                for (int e = 0; e < N_ENTITIES_TO_TRAIN; ++e) {
                    int entity = ENTITIES_TO_TRAIN[e];
                    int has_scorer = check_if_entity_has_component(
                        entity, SCORER_COMPONENT
                    );
                    int has_hidden_scorer
                        = check_if_entity_has_hidden_component(
                            entity, SCORER_COMPONENT
                        );
                    if (has_scorer == 0 && has_hidden_scorer == 0) {
                        fprintf(
                            stderr,
                            "ERROR: Trainable Entity %d doesn't have a "
                            "Scorer. It's a bug\n",
                            entity
                        );
                        exit(1);
                    }
                    Scorer* scorer = &SCENE.scorers[entity];
                    float score = get_total_score(scorer);

                    // TODO: Reset these values (max and min) on each
                    // generation. This will improve plots quality, as they
                    // will not expand infinitely
                    params->progress.min_score = min(
                        params->progress.min_score, score
                    );
                    params->progress.max_score = max(
                        params->progress.max_score, score
                    );
                    params->progress.episode_scores[e][episode] = score;
                    if (episode == 0
                        || score > get_total_score(&best_scorers[e])) {
                        best_scorers[e] = *scorer;
                    }
                    reset_scorer(scorer);
                }

                params->progress.episode = ++episode;
            }

            int n_elites = params->population.n_episodes
                           * params->evolution.elite_ratio;
            n_elites = max(2, n_elites);
            for (int e = 0; e < N_ENTITIES_TO_TRAIN; ++e) {
                static int indices[MAX_N_EPISODES];
                static Brain new_gen_brains[MAX_N_EPISODES];
                static float new_elite_streaks[MAX_N_EPISODES];

                // Don't create the new generation if the entity is
                // frozen. Just shuffle the elite
                if (params->progress.is_frozen[e] == 1) {
                    for (int i = 0; i < params->population.n_episodes;
                         ++i) {
                        int elite_idx = choose_idx(n_elites);
                        Brain* elite_brain = &GENERATION_ELITE_BRAINS[e]
                                                    [elite_idx];
                        Brain* gen_brain = &GENERATION_BRAINS[e][i]; 
                        clone_ptr_brain_into(gen_brain, elite_brain, 0);
                    }
                    memset(
                        BRAIN_ELITE_STREAKS[e],
                        0,
                        sizeof(BRAIN_ELITE_STREAKS[e])
                    );
                    continue;
                }

                // Prepare new brains generation
                float* gen_scores = params->progress.episode_scores[e];
                argsort(
                    gen_scores, indices, params->population.n_episodes, 1
                );

                int best_brain_saved = 0;
                for (int i = 0; i < params->population.n_episodes; ++i) {
                    int idx = indices[i];
                    if (i < n_elites) {
                        // Keep track the elite streak for the brains
                        new_elite_streaks[i] = BRAIN_ELITE_STREAKS[e][idx]
                                               + 1;
                        clone_ptr_brain_into(
                            &new_gen_brains[i],
                            &GENERATION_BRAINS[e][idx],
                            0
                        );
                        GENERATION_ELITE_BRAINS[e][i] = new_gen_brains[i];

                        float streak = new_elite_streaks[i];
                        float score = gen_scores[i];
                        Brain* brain = &new_gen_brains[i];
                        if (best_brain_saved == 0
                            && streak >= params->evolution.elite_streak) {
                            save_brain(brain->params.key, brain, &res_msg);
                            if (res_msg.flag != SUCCESS_RESULT) {
                                fprintf(
                                    stderr,
                                    "ERROR: Failed to save new best "
                                    "Brain: %s\n",
                                    res_msg.msg
                                );
                                exit(1);
                            }
                            printf(
                                "INFO: New best Brain with elite streak "
                                "%d and score %f saved for the "
                                "entity %d on the generation %d -> %s\n",
                                (int)streak,
                                score,
                                ENTITIES_TO_TRAIN[e],
                                generation,
                                brain->params.key
                            );
                            best_brain_saved = 1;
                        }
                    } else {
                        new_elite_streaks[i] = 0;
                    }

                    GENETIC_TRAINING->progress.elite_streaks[e][i]
                        = new_elite_streaks[i];
                    destroy_brain(&GENERATION_BRAINS[e][idx]);
                }

                // Perform crossover on the elite brains to obtain the next
                // new generation
                for (int i = n_elites; i < params->population.n_episodes;
                     ++i) {
                    Brain* brain1 = &new_gen_brains[choose_idx(n_elites)];
                    Brain* brain2 = &new_gen_brains[choose_idx(n_elites)];
                    new_gen_brains[i] = crossover_brains(
                        brain1,
                        brain2,
                        params->evolution.mutation_rate,
                        params->evolution.mutation_strength
                    );
                }

                memcpy(
                    GENERATION_BRAINS[e],
                    new_gen_brains,
                    sizeof(new_gen_brains)
                );
                memcpy(
                    BRAIN_ELITE_STREAKS[e],
                    new_elite_streaks,
                    sizeof(new_elite_streaks)
                );
            }

            params->progress.generation = ++generation;
            load_scene(".tmp.xscene", &res_msg);
        }

        exit(0);
    }
}

static void render_genetic_training_menu_bar(void) {
    if (igBeginMenu("Genetic Training", 1)) {
        if (igBeginMenu("File", 1)) {
            if (menu_item("Open (TODO: Not implemented)", "", false, 1)) {}
            if (menu_item(
                    "Save As (TODO: Not implemented)", "", false, 1
                )) {}
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

static void update_counters(void) {
    N_ENTITIES_WITHOUT_SCORER = 0;
    N_ENTITIES_TO_TRAIN = 0;
    for (int entity = 0; entity < SCENE.n_entities; ++entity) {
        if (!check_if_entity_has_component(entity, CONTROLLER_COMPONENT)) {
            continue;
        }

        Controller controller = SCENE.controllers[entity];
        ControllerType type = controller.type;
        if (type != BRAIN_AI_CONTROLLER) {
            continue;
        }

        char* key = controller.c.brain_ai.key;
        if (key[0] == '\0') {
            continue;
        }
        Brain* brain = get_or_load_brain(key);

        int has_scorer = check_if_entity_has_component(
            entity, SCORER_COMPONENT
        );
        if (brain->params.is_trainable) {
            if (has_scorer) {
                ENTITIES_TO_TRAIN[N_ENTITIES_TO_TRAIN++] = entity;
            } else {
                ENTITIES_WITHOUT_SCORERS[N_ENTITIES_WITHOUT_SCORER++]
                    = entity;
            }
        }
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
        if (igButton("Fix", IG_VEC2_ZERO)) {
            reset_scorer(scorer);
            SCENE.components[entity] |= SCORER_COMPONENT;
        }
    }
}

static void render_entities_to_train(void) {
    igText("%d Entities to train:", N_ENTITIES_TO_TRAIN);
    for (int e = 0; e < N_ENTITIES_TO_TRAIN; ++e) {
        int entity = ENTITIES_TO_TRAIN[e];
        static char str[16];
        sprintf(str, "  Entity: %d", entity);

        if (igBeginMenu(str, 1)) {
            igCheckbox(
                "is frozen",
                (bool*)&GENETIC_TRAINING->progress.is_frozen[e]
            );
            SimulationStatus status = GENETIC_TRAINING->progress.status;
            if (status == SIMULATION_NOT_STARTED) {
                Scorer* scorer = &SCENE.scorers[entity];
                render_scorer_weights_inspector(scorer);
            } else if (GENETIC_TRAINING->progress.episode > 0) {
                Scorer* scorer
                    = &GENETIC_TRAINING->progress.best_scorers[e];
                igText("Total: %.2f", get_total_score(scorer));
                igSeparator();
                render_scorer_values_inspector(scorer);
            }
            igEndMenu();
        }
    }
}

static void render_genetic_training_parameters(void) {
    int is_started = GENETIC_TRAINING->progress.status
                     == SIMULATION_NOT_STARTED;
    if (is_started) {
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
    }

    igText("Population:");
    ig_drag_float(
        "Episode time (s)",
        &GENETIC_TRAINING->population.episode_time,
        5.0,
        600.0,
        1.0,
        0
    );

    if (is_started) {
        ig_drag_int(
            "Episodes",
            &GENETIC_TRAINING->population.n_episodes,
            10,
            MAX_N_EPISODES,
            1,
            0
        );
    }

    igText("Evolution:");
    ig_drag_int(
        "Elite streak",
        &GENETIC_TRAINING->evolution.elite_streak,
        1,
        100,
        1,
        0
    );
    ig_drag_float(
        "Elite ratio",
        &GENETIC_TRAINING->evolution.elite_ratio,
        0.01,
        0.9,
        0.01,
        0
    );
    ig_drag_float(
        "Mutation rate",
        &GENETIC_TRAINING->evolution.mutation_rate,
        0.01,
        0.9,
        0.01,
        0
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
            int can_start = 1;

            if (N_ENTITIES_TO_TRAIN > MAX_N_ENTITIES_TO_TRAIN) {
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
            kill_genetic_training();
            reset_genetic_training();
        }
    }
}

static void render_evolution_progress_bar(void) {
    GeneticTraining* params = GENETIC_TRAINING;
    float fraction = (float)params->progress.episode
                     / params->population.n_episodes;
    ImVec2 size_arg = {0.0, 0.0};
    igProgressBar(fraction, size_arg, "");
    ig_same_line();
    igText("Generation: %d", params->progress.generation);

    fraction = (float)params->progress.episode_time
               / params->population.episode_time;
    igProgressBar(fraction, size_arg, "");
    ig_same_line();
    igText(
        "Episode: %d/%d",
        params->progress.episode,
        params->population.n_episodes
    );
}

// TODO: Bug! Plots are drawn for entities which are present in the
// main process. E.g if the entity in main process dies, it disappears
// from the genetic training process (or at lease genetic training plot)
static void render_evolution_plots(void) {
    ImPlotContext* ctx = ImPlot_CreateContext();
    ImPlot_SetCurrentContext(ctx);

    float min_score = GENETIC_TRAINING->progress.min_score;
    float max_score = GENETIC_TRAINING->progress.max_score;

    if (ImPlot_BeginPlot("Best scores", IG_VEC2_ZERO, 0)) {
        int offset = 0;
        int stride = sizeof(float);
        int n = GENERATIONS.length;
        float* xs = GENERATIONS.data;

        ImPlot_SetupAxis(ImAxis_X1, "Generation", 0);
        ImPlot_SetupAxis(ImAxis_Y1, "Score", 0);
        ImPlot_SetupAxisLimits(ImAxis_X1, 0.0, (int)(1.05 * n), 0);
        ImPlot_SetupAxisLimits(
            ImAxis_Y1,
            min_score - fabs(0.05 * min_score),
            max_score + fabs(0.05 * max_score),
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

    if (ImPlot_BeginPlot("Episode scores", IG_VEC2_ZERO, 0)) {
        ImPlot_SetupAxes("Score", "N Episoded", 0, 0);
        ImPlot_SetupAxisLimits(
            ImAxis_X1,
            min_score - fabs(0.05 * min_score),
            max_score + fabs(0.05 * max_score),
            0
        );
        ImPlot_SetupAxisLimits(ImAxis_Y1, 0.0, 50, 0);
        ImPlotRange range = {
            min_score - fabs(0.05 * min_score),
            max_score + fabs(0.05 * max_score)};

        int bins;
        double width;
        float bar_scale = 1.0;
        int episode = GENETIC_TRAINING->progress.episode;
        int hist_flags = 0;

        for (int e = 0; e < N_ENTITIES_TO_TRAIN; ++e) {
            float* scores = GENETIC_TRAINING->progress.episode_scores[e];
            char str[16];
            sprintf(str, "Entity: %d", ENTITIES_TO_TRAIN[e]);

            ImPlot_CalculateBins_FloatPtr(
                scores, episode, ImPlotBin_Sturges, range, &bins, &width
            );

            ImPlot_PlotHistogram_FloatPtr(
                str, scores, episode, bins, bar_scale, range, hist_flags
            );
        }

        ImPlot_EndPlot();
    }

    ig_same_line();
    if (ImPlot_BeginPlot("Elite streaks", IG_VEC2_ZERO, 0)) {
        ImPlot_SetupAxes("Streak", "N Episodes", 0, 0);
        ImPlot_SetupAxisLimits(ImAxis_X1, 1, 20, 0);
        ImPlot_SetupAxisLimits(ImAxis_Y1, 0, 5, 0);
        ImPlotRange range = {1, 20};

        float bar_scale = 1.0;
        int episode = GENETIC_TRAINING->progress.episode;
        int hist_flags = 0;
        for (int e = 0; e < N_ENTITIES_TO_TRAIN; ++e) {
            if (GENETIC_TRAINING->progress.is_frozen[e] == 1) {
                continue;
            }

            float* streaks = GENETIC_TRAINING->progress.elite_streaks[e];
            char str[16];
            sprintf(str, "Entity: %d", ENTITIES_TO_TRAIN[e]);
            ImPlot_PlotHistogram_FloatPtr(
                str, streaks, episode, 40, bar_scale, range, hist_flags
            );
        }

        ImPlot_EndPlot();
    }

    ImPlot_DestroyContext(ctx);
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
        igCheckbox("All immortal", (bool*)&DEBUG.gameplay.all_immortal);
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
