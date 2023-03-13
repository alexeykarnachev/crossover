#pragma once
#include "array.h"
#include "const.h"
#include "hashmap.h"

#define PROFILE_FUNC(func, ...) \
    profiler_push(PROFILER, #func); \
    func(__VA_ARGS__); \
    profiler_pop(PROFILER);

#define PROFILE_START(name) profiler_push(PROFILER, #name);

#define PROFILE_STOP() profiler_pop(PROFILER);

typedef enum SimulationStatus {
    SIMULATION_NOT_STARTED = 0,
    SIMULATION_RUNNING,
    SIMULATION_PAUSED
} SimulationStatus;

typedef struct ProfilerStage {
    double time;
    int n_calls;
    char name[MAX_PROFILER_STAGE_NAME_LENGTH];
} ProfilerStage;

typedef struct Profiler {
    struct {
        SimulationStatus status;
        HashMap stages_map;
        ProfilerStage stages_stack[MAX_PROFILER_STAGES_DEPTH];
        char stages_names[MAX_N_PROFILER_STAGES]
                         [MAX_PROFILER_STAGE_NAME_LENGTH];
        ProfilerStage stages_summary[MAX_N_PROFILER_STAGES];
        int n_stages;
        int depth;
    } progress;

    struct {
        float dt_ms;
    } simulation;

    char scene_file_path[MAX_PATH_LENGTH];
} Profiler;

extern Profiler* PROFILER;
extern int PROFILER_ENABLED;

void reset_profiler(Profiler* profiler);
void init_profiler(Profiler* profiler);
void destroy_profiler(Profiler* profiler);

void profiler_push(Profiler* profiler, char* name);
void profiler_pop(Profiler* profiler);
