#include "../genetic_training.h"

#include "../array.h"
#include "../scene.h"
#include "../utils.h"
#include "signal.h"
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <unistd.h>

GeneticTraining* GENETIC_TRAINING;
pid_t GENETIC_TRAINING_PID = -1;
static int GENETIC_TRAINING_SHMID;

static Array SCORES[MAX_N_ENTITIES_TO_TRAIN];

static float BRAIN_ELITE_STREAKS[MAX_N_ENTITIES_TO_TRAIN][MAX_N_EPISODES];
static Brain GENERATION_ELITE_BRAINS[MAX_N_ENTITIES_TO_TRAIN]
                                    [MAX_N_EPISODES];
static int GENERATION_N_ELITE_BRAINS[MAX_N_ENTITIES_TO_TRAIN];
static Brain GENERATION_BRAINS[MAX_N_ENTITIES_TO_TRAIN][MAX_N_EPISODES];

static int GENETIC_TRAINING_INITIALIZED = 0;
static int GENETIC_TRAINING_DESTROYED = 0;

void reset_genetic_training(void) {
    GeneticTraining* training = GENETIC_TRAINING;

    memset(&training->progress, 0, sizeof(training->progress));

    // TODO: Move these array to the GeneticTraining structure
    for (int e = 0; e < training->progress.n_entities_to_train; ++e) {
        SCORES[e].length = 0;
    }

    training->simulation.dt_ms = 17.0;

    training->population.episode_time = 100.0;
    training->population.n_episodes = 100;

    training->evolution.elite_streak = 10;
    training->evolution.elite_ratio = 0.15;
    training->evolution.mutation_rate = 0.3;
    training->evolution.mutation_strength = 0.15;
}

void init_genetic_training(void) {
    if (GENETIC_TRAINING_INITIALIZED == 1) {
        fprintf(
            stderr,
            "ERROR: GENETIC_TRAINING could be initialized only once\n"
        );
        exit(1);
    }

    GENETIC_TRAINING_SHMID = shmget(
        IPC_PRIVATE, sizeof(GeneticTraining), 0666 | IPC_CREAT
    );
    if (GENETIC_TRAINING_SHMID == -1) {
        perror("ERROR: Failed to create shared memory segment for the "
               "GeneticTraining\n");
        exit(1);
    }

    GENETIC_TRAINING = (GeneticTraining*)shmat(
        GENETIC_TRAINING_SHMID, NULL, 0
    );

    if (GENETIC_TRAINING == (GeneticTraining*)-1) {
        perror("ERROR: Failed to create shared memory segment for the "
               "GeneticTraining\n");
        exit(1);
    }

    memset(GENETIC_TRAINING, 0, sizeof(GeneticTraining));
    reset_genetic_training();

    GENETIC_TRAINING_INITIALIZED = 1;
    printf("DEBUG: GENETIC_TRAINING initialized\n");
}

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

    // TODO: Move these array to the GeneticTraining structure
    for (int e = 0; e < MAX_N_ENTITIES_TO_TRAIN; ++e) {
        if (SCORES[e].data != NULL) {
            destroy_array(&SCORES[e]);
        }
    }

    shmdt(GENETIC_TRAINING);
    shmctl(GENETIC_TRAINING_SHMID, IPC_RMID, NULL);

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

void start_genetic_training(void) {
    GENETIC_TRAINING_PID = fork();
    if (GENETIC_TRAINING_PID == -1) {
        perror("ERROR: Can't start Genetic Training\n");
    } else if (GENETIC_TRAINING_PID == 0) {
        ResultMessage res_msg = {0};
        save_scene(".tmp.xscene", &res_msg);

        GeneticTraining* params = GENETIC_TRAINING;
        SimulationStatus* status = &params->progress.status;
        *status = SIMULATION_RUNNING;

        for (int e = 0; e < params->progress.n_entities_to_train; ++e) {
            int entity = params->progress.entities_to_train[e];
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
            int n_entities_to_train = params->progress.n_entities_to_train;
            int* entities_to_train = params->progress.entities_to_train;
            while (episode < params->population.n_episodes) {
                for (int e = 0; e < n_entities_to_train; ++e) {
                    reset_scorer(&SCENE.scorers[entities_to_train[e]]);
                    int entity = entities_to_train[e];
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

                for (int e = 0; e < n_entities_to_train; ++e) {
                    int entity = entities_to_train[e];
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
            for (int e = 0; e < n_entities_to_train; ++e) {
                static int indices[MAX_N_EPISODES];
                static Brain new_gen_brains[MAX_N_EPISODES];
                static float new_elite_streaks[MAX_N_EPISODES];

                // Don't create the new generation if the entity is
                // frozen, just shuffle the elite
                if (params->progress.is_frozen[e] == 1) {
                    int n_elites = min(
                        GENERATION_N_ELITE_BRAINS[e], n_elites
                    );
                    for (int i = 0; i < params->population.n_episodes;
                         ++i) {
                        int elite_idx = choose_idx(n_elites);
                        Brain* elite_brain
                            = &GENERATION_ELITE_BRAINS[e][elite_idx];
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
                GENERATION_N_ELITE_BRAINS[e] = n_elites;
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
                                entities_to_train[e],
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
