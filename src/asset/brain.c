#include "brain.h"

#include "../component.h"
#include "../const.h"
#include "../utils.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BRAIN_INPUT_TYPE_ERROR(fn_name, type) \
    do { \
        fprintf( \
            stderr, \
            "ERROR: can't %s for the brain input with type id: %d. " \
            "Needs " \
            "to be implemented\n", \
            fn_name, \
            type \
        ); \
        exit(1); \
    } while (0)

#define BRAIN_OUTPUT_TYPE_ERROR(fn_name, type) \
    do { \
        fprintf( \
            stderr, \
            "ERROR: can't %s for the brain output with type id: %d. " \
            "Needs " \
            "to be implemented\n", \
            fn_name, \
            type \
        ); \
        exit(1); \
    } while (0)

Brain BRAINS[BRAINS_ARRAY_CAPACITY] = {0};
int N_BRAINS = 0;

void destroy_brain(Brain* brain) {
    if (brain->weights == NULL) {
        fprintf(stderr, "ERROR: Can't destroy Brain with NULL weights\n");
        exit(1);
    }
    free(brain->weights);
    memset(brain, 0, sizeof(Brain));
}

void destroy_brains(void) {
    int n_brains = 0;
    for (int i = 0; i < BRAINS_ARRAY_CAPACITY; ++i) {
        Brain* brain = &BRAINS[i];
        if (brain->params.key[0] == '\0') {
            if (brain->weights != NULL) {
                fprintf(
                    stderr,
                    "ERROR: Trying to destroy a Brain with null key, but "
                    "not null weights. It's a bug\n"
                );
                exit(1);
            }
            continue;
        }

        destroy_brain(brain);
        n_brains += 1;
    }

    if (n_brains != N_BRAINS) {
        fprintf(
            stderr,
            "ERROR: Number of destroyed Brains (%d) is not equal to the "
            "number "
            "of "
            "Brains registered in the engine runtime (%d). It's a bug\n",
            n_brains,
            N_BRAINS
        );
        exit(1);
    }

    N_BRAINS = 0;
}

// --------------------------------------------------------
// Brain general
Brain* add_brain_clone(Brain* brain, int allow_replacement) {
    if (N_BRAINS++ >= MAX_N_BRAINS) {
        fprintf(stderr, "ERROR: Can't initalize more Brains\n");
        exit(1);
    }

    BrainParams params = brain->params;

    if (params.key[0] == '\0') {
        fprintf(
            stderr,
            "ERROR: Can't initialize the Brain without specified key in "
            "its parameters\n"
        );
        exit(1);
    }

    uint64_t hash = get_bytes_hash(params.key, strlen(params.key));
    int idx = hash % BRAINS_ARRAY_CAPACITY;
    int idx_is_occupied = BRAINS[idx].params.key[0] != '\0';
    while (BRAINS[idx].params.key[0] != '\0') {
        int key_exists = strcmp(BRAINS[idx].params.key, params.key) == 0;
        if (key_exists && !allow_replacement) {
            fprintf(
                stderr,
                "ERROR: Brain with this key has been already initialized: "
                "%s. This could happen if you have several Brain files "
                "with different file names, but the internal Brain keys "
                "which are written in the files are different. You can "
                "set `allow_replacement`=true to replace "
                "existing Brain with the new one\n",
                BRAINS[idx].params.key
            );
            exit(1);
        } else if (key_exists && allow_replacement) {
            destroy_brain(&BRAINS[idx]);
            N_BRAINS -= 1;
            break;
        } else {
            idx = (idx + 1) % BRAINS_ARRAY_CAPACITY;
        }
    }

    BRAINS[idx] = *brain;
    int n_bytes = get_brain_size(brain->params) * sizeof(float);
    float* new_weights = malloc(n_bytes);
    memcpy(new_weights, brain->weights, n_bytes);
    BRAINS[idx].weights = new_weights;
    return &BRAINS[idx];
}

Brain init_local_brain(BrainParams params) {
    int n_weights = get_brain_size(params);
    int weights_n_bytes = sizeof(float) * n_weights;
    float* weights = (float*)malloc(weights_n_bytes);
    for (int i = 0; i < n_weights; ++i) {
        weights[i] = ((float)rand() / RAND_MAX) * 2.0 - 1.0;
        // TODO: Implement more clever weights initialization
        weights[i] *= 0.3;
    }

    Brain brain = {.params = params, .weights = weights};
    return brain;
}

Brain* init_brain(BrainParams params) {
    Brain local_brain = init_local_brain(params);
    Brain* brain = add_brain_clone(&local_brain, 0);
    destroy_brain(&local_brain);
    return brain;
}

Brain* load_brain(
    char* file_path, ResultMessage* res_msg, int allow_replacement
) {
    Brain local_brain = load_local_brain(file_path, res_msg);
    Brain* brain = add_brain_clone(&local_brain, allow_replacement);
    destroy_brain(&local_brain);
    return brain;
}

void reload_all_brains(ResultMessage* res_msg) {
    res_msg->flag = FAIL_RESULT;

    int n_brains_realoded = 0;
    for (int i = 0; i < BRAINS_ARRAY_CAPACITY; ++i) {
        Brain* brain = &BRAINS[i];
        char* file_path = brain->params.key;
        if (file_path[0] != '\0') {
            load_brain(file_path, res_msg, 1);
            n_brains_realoded += 1;
        }
    }

    if (n_brains_realoded != N_BRAINS) {
        fprintf(
            stderr,
            "ERROR: Expecting to reload %d Brains, but %d realoded. This "
            "is a bug\n",
            N_BRAINS,
            n_brains_realoded
        );
        exit(1);
    }

    static char str[64];
    sprintf(str, "INFO: %d Brains reloaded", n_brains_realoded);
    res_msg->flag = SUCCESS_RESULT;
    strcpy(res_msg->msg, str);
}

Brain* get_brain(char* key, int allow_null) {
    // TODO: Reuse HashMap struct here
    uint64_t hash = get_bytes_hash(key, strlen(key));
    int idx = hash % BRAINS_ARRAY_CAPACITY;
    while (strlen(BRAINS[idx].params.key) != 0) {
        if (strcmp(BRAINS[idx].params.key, key) == 0) {
            return &BRAINS[idx];
        }
        idx = (idx + 1) % BRAINS_ARRAY_CAPACITY;
    }

    if (!allow_null) {
        fprintf(
            stderr,
            "ERROR: Can't get the Brain with key: '%s' since it is not "
            "loaded\n",
            key
        );
        exit(1);
    }

    return NULL;
}

Brain* get_or_load_brain(char* key) {
    Brain* brain = get_brain(key, 1);
    if (brain == NULL) {
        ResultMessage res_msg = {0};
        brain = load_brain(key, &res_msg, 0);
    }
    return brain;
}

Brain* clone_brain(char* dst_key, char* src_key, int randomize_weights) {
    Brain* src_brain = get_brain(src_key, 0);
    if (get_brain(dst_key, 1) != NULL) {
        fprintf(
            stderr,
            "ERROR: Can't copy the Brain with the dst_key='%s' since this "
            "destination key is already occupied by another Brain\n",
            dst_key
        );
        exit(1);
    }

    BrainParams params = src_brain->params;
    strcpy(params.key, dst_key);
    Brain* dst_brain = init_brain(params);
    if (!randomize_weights) {
        int n_bytes = get_brain_size(params) * sizeof(float);
        memcpy(dst_brain->weights, src_brain->weights, n_bytes);
    }
    return dst_brain;
}

void clone_ptr_brain_into(
    Brain* dst_brain, Brain* src_brain, int randomize_weights
) {
    BrainParams params = src_brain->params;
    *dst_brain = init_local_brain(params);
    if (!randomize_weights) {
        int n_bytes = get_brain_size(params) * sizeof(float);
        memcpy(dst_brain->weights, src_brain->weights, n_bytes);
    }
}

void clone_key_brain_into(
    Brain* dst_brain, char* src_key, int randomize_weights
) {
    Brain* src_brain = get_brain(src_key, 0);
    clone_ptr_brain_into(dst_brain, src_brain, randomize_weights);
}

void randomize_brain(Brain* brain) {
    BrainParams params = brain->params;
    int n_weights = get_brain_size(params);
    for (int i = 0; i < n_weights; ++i) {
        brain->weights[i] = ((float)rand() / RAND_MAX) * 2.0 - 1.0;
    }
}

Brain load_local_brain(char* file_path, ResultMessage* res_msg) {
    FILE* fp = open_file(file_path, res_msg, "rb");
    if (res_msg->flag != SUCCESS_RESULT) {
        fprintf(
            stderr, "ERROR: Failed to load the Brain: %s\n", file_path
        );
        exit(1);
    }

    int version;
    fread(&version, sizeof(int), 1, fp);
    if (version != BRAIN_VERSION) {
        sprintf(
            res_msg->msg,
            "ERROR: Brain version %d is not compatible with the engine, "
            "expecting the version %d\n",
            version,
            BRAIN_VERSION
        );
        exit(1);
    }

    BrainParams params;
    fread(&params, sizeof(BrainParams), 1, fp);
    Brain brain = init_local_brain(params);
    int n_weights = get_brain_size(params);
    fread(brain.weights, sizeof(float), n_weights, fp);

    fclose(fp);
    res_msg->flag = SUCCESS_RESULT;

    sprintf(res_msg->msg, "INFO: Brain is loaded");
    return brain;
}

void save_brain(char* file_path, Brain* brain, ResultMessage* res_msg) {
    FILE* fp = open_file(file_path, res_msg, "wb");
    if (res_msg->flag != SUCCESS_RESULT) {
        return;
    }

    float* weights = brain->weights;
    int n_weights = get_brain_size(brain->params);

    if (weights == NULL || n_weights == 0) {
        res_msg->flag = FAIL_RESULT;
        strcpy(
            res_msg->msg, "ERROR: Can't save the Brain without weights"
        );
        return;
    }

    int version = BRAIN_VERSION;
    fwrite(&version, sizeof(int), 1, fp);
    fwrite(&brain->params, sizeof(BrainParams), 1, fp);
    fwrite(weights, sizeof(float), n_weights, fp);

    fclose(fp);
    res_msg->flag = SUCCESS_RESULT;

    sprintf(res_msg->msg, "INFO: Brain is saved");
    return;
}

uint64_t get_brain_required_input_types(BrainParams params) {
    uint64_t components = 0;
    for (int i = 0; i < params.n_inputs; ++i) {
        BrainInputType type = params.inputs[i].type;
        switch (type) {
            case TARGET_ENTITY_INPUT: {
                components |= VISION_COMPONENT;
                break;
            }
            case TARGET_DISTANCE_INPUT: {
                components |= VISION_COMPONENT;
                break;
            }
            case SELF_HEALTH_INPUT: {
                components |= HEALTH_COMPONENT;
                break;
            }
            case SELF_SPEED_INPUT: {
                components |= KINEMATIC_MOVEMENT_COMPONENT;
                break;
            }
            default:
                BRAIN_INPUT_TYPE_ERROR(
                    "get_brain_required_components", type
                );
        }
    }

    return components;
}

int get_brain_size(BrainParams params) {
    int input_size = get_brain_input_size(params);
    int output_size = get_brain_output_size(params);
    if (!input_size || !output_size) {
        return 0;
    }

    int n_layers = params.n_layers;
    int* layer_sizes = params.layer_sizes;
    int inp_layer_size = input_size;
    int out_layer_size;
    int n_weights = 0;
    for (int i = 0; i < n_layers + 1; ++i) {
        out_layer_size = i >= n_layers ? output_size : layer_sizes[i];
        n_weights += (inp_layer_size + 1) * out_layer_size;
        inp_layer_size = out_layer_size;
    }

    return n_weights;
}

Brain crossover_brains(
    Brain* brain0,
    Brain* brain1,
    float mutation_rate,
    float mutation_strength
) {
    int n_weights0 = get_brain_size(brain0->params);
    int n_weights1 = get_brain_size(brain1->params);
    if (n_weights0 != n_weights1) {
        fprintf(
            stderr,
            "ERROR: Can't crossover Brains with different number of "
            "weights: %d != %d\n",
            n_weights0,
            n_weights1
        );
        exit(1);
    }

    Brain brain = init_local_brain(brain0->params);
    for (int i = 0; i < n_weights0; ++i) {
        float weight = frand01() < 0.5 ? brain0->weights[i]
                                       : brain1->weights[i];
        if (frand01() < mutation_rate) {
            weight += (frand01() * 2.0 - 1.0) * mutation_strength;
        }
        brain.weights[i] = weight;
    }

    return brain;
}

// --------------------------------------------------------
// Brain inputs
BrainInputType BRAIN_INPUT_TYPES[N_BRAIN_INPUT_TYPES] = {
    TARGET_ENTITY_INPUT,
    TARGET_DISTANCE_INPUT,
    SELF_HEALTH_INPUT,
    SELF_SPEED_INPUT};
const char* BRAIN_INPUT_TYPE_NAMES[N_BRAIN_INPUT_TYPES] = {
    "Entity (target)",
    "Distance (target)",
    "Health (self)",
    "Speed (self)"};

BrainInput init_target_entity_brain_input(void) {
    BrainInput input;
    input.type = TARGET_ENTITY_INPUT;

    TargetEntityBrainInput target_entity = {0};
    input.i.target_entity = target_entity;
    return input;
}

BrainInput init_target_distance_brain_input(void) {
    BrainInput input;
    input.type = TARGET_DISTANCE_INPUT;

    TargetDistanceBrainInput target_distance;
    input.i.target_distance = target_distance;
    return input;
}

BrainInput init_self_health_brain_input(void) {
    BrainInput input;
    input.type = SELF_HEALTH_INPUT;

    SelfHealthBrainInput self_health;
    input.i.self_health = self_health;
    return input;
}

BrainInput init_self_speed_brain_input(void) {
    BrainInput input;
    input.type = SELF_SPEED_INPUT;

    SelfSpeedBrainInput self_speed;
    input.i.self_speed = self_speed;
    return input;
}

void change_brain_input_type(
    BrainInput* brain_input, BrainInputType target_type
) {
    BrainInputType source_type = brain_input->type;
    if (source_type == target_type) {
        return;
    }

    switch (target_type) {
        case TARGET_ENTITY_INPUT:
            *brain_input = init_target_entity_brain_input();
            break;
        case TARGET_DISTANCE_INPUT:
            *brain_input = init_target_distance_brain_input();
            break;
        case SELF_HEALTH_INPUT:
            *brain_input = init_self_health_brain_input();
            break;
        case SELF_SPEED_INPUT:
            *brain_input = init_self_speed_brain_input();
            break;
        default:
            BRAIN_INPUT_TYPE_ERROR("change_brain_input_type", source_type);
    }
}

int get_brain_input_size(BrainParams params) {
    int n_view_rays = params.n_view_rays;
    int input_size = 0;
    for (int i = 0; i < params.n_inputs; ++i) {
        BrainInput input = params.inputs[i];
        BrainInputType type = input.type;
        switch (type) {
            case TARGET_ENTITY_INPUT:
                input_size += n_view_rays;
                break;
            case TARGET_DISTANCE_INPUT:
                input_size += n_view_rays;
                break;
            case SELF_HEALTH_INPUT:
                input_size += 1;
                break;
            case SELF_SPEED_INPUT:
                input_size += 1;
                break;
            default:
                BRAIN_INPUT_TYPE_ERROR("get_brain_input_size", type);
        }
    }

    return input_size;
}

// --------------------------------------------------------
// Brain outputs
BrainInputType BRAIN_OUTPUT_TYPES[N_BRAIN_OUTPUT_TYPES] = {
    WATCH_ORIENTATION_OUTPUT,
    MOVE_ORIENTATION_OUTPUT,
    IS_SHOOTING_OUTPUT,
    IS_MOVING_OUTPUT};
const char* BRAIN_OUTPUT_TYPE_NAMES[N_BRAIN_OUTPUT_TYPES] = {
    "Look at orientation", "Move orientation", "Is shooting", "Is moving"};

BrainOutput init_watch_orientation_brain_output(void) {
    BrainOutput output;
    output.type = WATCH_ORIENTATION_OUTPUT;

    WatchOrientationBrainOutput watch_orientation;
    output.o.watch_orientation = watch_orientation;
    return output;
}

BrainOutput init_move_orientation_brain_output(void) {
    BrainOutput output;
    output.type = MOVE_ORIENTATION_OUTPUT;

    MoveOrientationBrainOutput move_orientation = {.n_directions = 8};
    output.o.move_orientation = move_orientation;
    return output;
}

BrainOutput init_is_shooting_brain_output(void) {
    BrainOutput output;
    output.type = IS_SHOOTING_OUTPUT;

    IsShootingBrainOutput is_shooting;
    output.o.is_shooting = is_shooting;
    return output;
}

BrainOutput init_is_moving_brain_output(void) {
    BrainOutput output;
    output.type = IS_MOVING_OUTPUT;

    IsMovingBrainOutput is_moving;
    output.o.is_moving = is_moving;
    return output;
}

void change_brain_output_type(
    BrainOutput* brain_output, BrainOutputType target_type
) {
    BrainOutputType source_type = brain_output->type;
    if (source_type == target_type) {
        return;
    }

    switch (target_type) {
        case WATCH_ORIENTATION_OUTPUT:
            *brain_output = init_watch_orientation_brain_output();
            break;
        case MOVE_ORIENTATION_OUTPUT:
            *brain_output = init_move_orientation_brain_output();
            break;
        case IS_SHOOTING_OUTPUT:
            *brain_output = init_is_shooting_brain_output();
            break;
        case IS_MOVING_OUTPUT:
            *brain_output = init_is_moving_brain_output();
            break;
        default:
            BRAIN_INPUT_TYPE_ERROR(
                "change_brain_output_type", source_type
            );
    }
}

int get_brain_output_size(BrainParams params) {
    int n_view_rays = params.n_view_rays;
    int output_size = 0;
    for (int i = 0; i < params.n_outputs; ++i) {
        BrainOutput output = params.outputs[i];
        BrainOutputType type = output.type;
        switch (type) {
            case WATCH_ORIENTATION_OUTPUT:
                output_size += n_view_rays;
                break;
            case MOVE_ORIENTATION_OUTPUT:
                output_size += output.o.move_orientation.n_directions;
                break;
            case IS_SHOOTING_OUTPUT:
                output_size += 1;
                break;
            case IS_MOVING_OUTPUT:
                output_size += 1;
                break;
            default:
                BRAIN_INPUT_TYPE_ERROR("get_brain_output_size", type);
        }
    }

    return output_size;
}
