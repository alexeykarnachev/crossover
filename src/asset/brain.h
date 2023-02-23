#pragma once
#include "../const.h"
#include "../utils.h"
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

typedef enum BrainOutputType {
    WATCH_ORIENTATION_OUTPUT,
    MOVE_ORIENTATION_OUTPUT,
    IS_SHOOTING_OUTPUT,
    IS_MOVING_OUTPUT
} BrainOutputType;

typedef struct WatchOrientationBrainOutput {
} WatchOrientationBrainOutput;

typedef struct MoveOrientationBrainOutput {
    int n_directions;
} MoveOrientationBrainOutput;

typedef struct IsShootingBrainOutput {
} IsShootingBrainOutput;

typedef struct IsMovingBrainOutput {
} IsMovingBrainOutput;

typedef struct BrainOutput {
    BrainOutputType type;
    union {
        WatchOrientationBrainOutput watch_orientation;
        MoveOrientationBrainOutput move_orientation;
        IsShootingBrainOutput is_shooting;
        IsMovingBrainOutput is_moving;
    } o;
} BrainOutput;

void change_brain_output_type(
    BrainOutput* brain_output, BrainOutputType target_type
);

#define N_BRAIN_OUTPUT_TYPES 4
BrainInputType BRAIN_OUTPUT_TYPES[N_BRAIN_OUTPUT_TYPES];
const char* BRAIN_OUTPUT_TYPE_NAMES[N_BRAIN_OUTPUT_TYPES];

typedef struct BrainParams {
    char key[MAX_PATH_LENGTH];
    BrainInput inputs[MAX_N_BRAIN_INPUTS];
    BrainOutput outputs[MAX_N_BRAIN_OUTPUTS];
    int layer_sizes[MAX_N_BRAIN_LAYERS];
    int n_view_rays;
    int n_inputs;
    int n_outputs;
    int n_layers;
    int is_trainable;
} BrainParams;

typedef struct Brain {
    BrainParams params;
    float* weights;
} Brain;

extern Brain BRAINS[BRAINS_ARRAY_CAPACITY];
extern int N_BRAINS;

Brain init_local_brain(BrainParams params);
Brain* init_brain(BrainParams params);
void randomize_brain(Brain* brain);
Brain* add_brain_clone(Brain* brain, int allow_replacement);
void destroy_brains(void);
void destroy_brain(Brain* brain);
void save_brain(char* file_path, Brain* brain, ResultMessage* res_msg);
BrainParams load_brain_params(char* file_path, ResultMessage* res_msg);
Brain load_local_brain(char* file_path, ResultMessage* res_msg);
Brain* load_brain(char* file_path, ResultMessage* res_msg);
Brain* get_brain(char* key, int allow_null);
Brain* get_or_load_brain(char* key);
Brain* clone_brain(char* dst_key, char* src_key, int randomize_weights);
Brain crossover_brains(
    Brain* brain0,
    Brain* brain1,
    float mutation_rate,
    float mutation_strength
);
void clone_key_brain_into(
    Brain* dst_brain, char* src_key, int randomize_weights
);
void clone_ptr_brain_into(
    Brain* dst_brain, Brain* src_brain, int randomize_weights
);

int get_brain_input_size(BrainParams params);
int get_brain_output_size(BrainParams params);
int get_brain_size(BrainParams params);
uint64_t get_brain_required_input_types(BrainParams params);
uint64_t get_brain_required_output_types(BrainParams params);

BrainInput init_target_entity_brain_input(void);
BrainInput init_target_distance_brain_input(void);
BrainInput init_target_health_brain_input(void);
BrainInput init_self_health_brain_input(void);

BrainOutput init_watch_orientation_brain_output(void);
BrainOutput init_move_orientation_brain_output(void);
BrainOutput init_is_shooting_brain_output(void);
