#pragma once
#include "component.h"
#include "const.h"
#include "profiler.h"
#include "signal.h"

typedef struct GeneticTraining {
    struct {
        int is_frozen[MAX_N_ENTITIES_TO_TRAIN];
        int entities_to_train[MAX_N_ENTITIES_TO_TRAIN];
        Scorer best_scorers[MAX_N_ENTITIES_TO_TRAIN];
        float episode_scores[MAX_N_ENTITIES_TO_TRAIN][MAX_N_EPISODES];
        float elite_streaks[MAX_N_ENTITIES_TO_TRAIN][MAX_N_EPISODES];

        SimulationStatus status;
        int n_entities_to_train;
        int generation;
        int episode;
        float episode_time;
    } progress;

    struct {
        float dt_ms;
    } simulation;

    struct {
        float episode_time;
        int n_episodes;
    } population;

    struct {
        int elite_streak;
        float elite_ratio;
        float mutation_rate;
        float mutation_strength;
    } evolution;
} GeneticTraining;

extern GeneticTraining* GENETIC_TRAINING;
extern pid_t GENETIC_TRAINING_PID;

void init_genetic_training(void);
void destroy_genetic_training(void);
void kill_genetic_training(void);

void start_genetic_training(void);
void reset_genetic_training(void);
