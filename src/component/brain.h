#pragma once
#include "../const.h"
#include <stdint.h>

typedef enum BrainInputType {
    // Vision inputs
    IS_OBSTACLE_INPUT = 1 << 0,
    IS_ENEMY_INPUT = 1 << 1,
    DISTANCE_INPUT = 1 << 2

    //
} BrainInputType;

#define N_BRAIN_INPUT_TYPES 3
BrainInputType BRAIN_INPUT_TYPES[N_BRAIN_INPUT_TYPES];

#define N_MOVE_DIRECTIONS 8

typedef enum BrainOutputType {
    MOVE_DIRECTION_OUTPUT = 1 << 0,
    IS_SHOOT_OUTPUT = 1 << 1
} BrainOutputType;

#define N_BRAIN_OUTPUT_TYPES 2
BrainInputType BRAIN_OUTPUT_TYPES[N_BRAIN_OUTPUT_TYPES];

typedef struct Brain {
    int layer_sizes[MAX_N_BRAIN_LAYERS];
    int n_layers;
    int input_size;
    int output_size;
    float* weights;
} Brain;

Brain init_brain(
    int entity,
    int layer_sizes[MAX_N_BRAIN_LAYERS],
    int n_layers,
    uint64_t input_types,
    uint64_t output_types
);
