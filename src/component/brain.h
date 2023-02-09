#pragma once
#include "../const.h"
#include <stdint.h>

typedef enum BrainInputType {
    TARGET_ENTITY_INPUT,
    TARGET_DISTANCE_INPUT,
    SELF_HEALTH_INPUT
} BrainInputType;

typedef struct TargetEntityBrainInput {
    uint64_t components;
} TargetEntityBrainInput;

typedef struct TargetDistanceBrainInput {
} TargetDistanceBrainInput;

typedef struct TargetHealthBrainInput {
} TargetHealthBrainInput;

typedef struct SelfHealthBrainInput {
} SelfHealthBrainInput;

typedef struct BrainInput {
    BrainInputType type;
    union {
        TargetEntityBrainInput target_entity;
        TargetDistanceBrainInput target_distance;
        TargetHealthBrainInput target_health;
        SelfHealthBrainInput self_health;
    } i;
} BrainInput;

void change_brain_input_type(
    BrainInput* brain_input, BrainInputType target_type
);

#define N_BRAIN_INPUT_TYPES 3
BrainInputType BRAIN_INPUT_TYPES[N_BRAIN_INPUT_TYPES];
const char* BRAIN_INPUT_TYPE_NAMES[N_BRAIN_INPUT_TYPES];

#define N_MOVE_DIRECTIONS 8

typedef enum BrainOutputType {
    MOVE_DIRECTION_OUTPUT = 1 << 0,
    IS_SHOOT_OUTPUT = 1 << 1
} BrainOutputType;

#define N_BRAIN_OUTPUT_TYPES 2
BrainInputType BRAIN_OUTPUT_TYPES[N_BRAIN_OUTPUT_TYPES];

typedef struct Brain {
    BrainInput inputs[MAX_N_BRAIN_INPUTS];
    int layer_sizes[MAX_N_BRAIN_LAYERS];
    int n_inputs;
    int n_layers;
    int input_size;
    int output_size;
    float* weights;
} Brain;

int get_brain_input_size(Brain brain, int n_view_rays);
uint64_t get_brain_required_component_types(Brain brain);

BrainInput init_target_entity_brain_input(void);
BrainInput init_target_distance_brain_input(void);
BrainInput init_target_health_brain_input(void);
BrainInput init_self_health_brain_input(void);
