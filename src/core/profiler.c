#include "../profiler.h"

#include "../utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int PROFILER_ENABLED = 0;
Profiler *PROFILER;

void init_profiler(Profiler *profiler) {
    memset(profiler, 0, sizeof(Profiler));
    profiler->simulation.dt_ms = 17.0;
    profiler->progress.status = SIMULATION_NOT_STARTED;
    profiler->progress.stages_map = init_hashmap();
    profiler->progress.depth = 0;
    printf("DEBUG: PROFILER initialized\n");
}

void reset_profiler(Profiler *profiler) {
    HashMap *stages_map = &profiler->progress.stages_map;
    for (int i = 0; i < stages_map->capacity; ++i) {
        void *stage = stages_map->items[i].value;
        if (stage != NULL) {
            free(stage);
        }
        stages_map->items[i].value = NULL;
    }

    stages_map->length = 0;
    profiler->progress.depth = 0;
    profiler->progress.n_stages = 0;
    profiler->progress.status = SIMULATION_NOT_STARTED;
    printf("DEBUG: PROFILER reset\n");
}

void destroy_profiler(Profiler *profiler) {
    reset_profiler(profiler);
    destroy_hashmap(&profiler->progress.stages_map);
    printf("DEBUG: PROFILER destroyed\n");
}

// TODO: Add ENABLE_PROFILER define macro which enables (if set)
// this function, otherwise the profiler is disabled and doesn't affect
// the application performance
void profiler_push(Profiler *profiler, char *name) {
    if (PROFILER_ENABLED != 1) {
        return;
    }

    if (profiler->progress.depth == MAX_PROFILER_STAGES_DEPTH) {
        fprintf(
            stderr,
            "ERROR: The maximum profiling depth is reached: %d. Can't "
            "profile any deeper\n",
            MAX_PROFILER_STAGES_DEPTH
        );
        exit(1);
    }

    ProfilerStage stage = {0};
    stage.time = get_curr_time();

    // TODO: Add check for not exceeding the MAX_PROFILER_STAGE_NAME_LENGTH
    int curr_name_length = 0;
    if (profiler->progress.depth > 0) {
        char *prev_stage_name
            = profiler->progress.stages_stack[profiler->progress.depth - 1].name;
        strcpy(stage.name, prev_stage_name);
        curr_name_length = strlen(prev_stage_name);
        stage.name[curr_name_length++] = '.';
    }
    strcpy(&stage.name[curr_name_length], name);

    profiler->progress.stages_stack[profiler->progress.depth++] = stage;
}

void profiler_pop(Profiler *profiler) {
    if (PROFILER_ENABLED != 1) {
        return;
    }

    if (profiler->progress.depth == 0) {
        fprintf(stderr, "ERROR: Can't pop a stage from the empty Profiler stack\n");
        exit(1);
    }

    int depth = --profiler->progress.depth;
    ProfilerStage curr_stage = profiler->progress.stages_stack[depth];
    double curr_time = get_curr_time();

    HashMap *stages_map = &profiler->progress.stages_map;
    ProfilerStage *stage = (ProfilerStage *)hashmap_try_get(stages_map, curr_stage.name);
    if (stage == NULL) {
        stage = (ProfilerStage *)malloc(sizeof(ProfilerStage));
        memcpy(stage, &curr_stage, sizeof(ProfilerStage));
        stage->time = 0;
        hashmap_put(stages_map, stage->name, (void *)stage);
        // TODO: Check that profiler->n_stages doesn't exceed
        // the `MAX_N_PROFILER_STAGES`
        strcpy(
            profiler->progress.stages_names[profiler->progress.n_stages++], stage->name
        );
    }
    stage->time += curr_time - curr_stage.time;
    stage->n_calls += 1;
}
