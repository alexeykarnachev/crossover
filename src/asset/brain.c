#include "brain.h"

#include "../component.h"
#include "../const.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// --------------------------------------------------------
// Brain inputs
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

BrainInputType BRAIN_INPUT_TYPES[N_BRAIN_INPUT_TYPES] = {
    TARGET_ENTITY_INPUT, TARGET_DISTANCE_INPUT, SELF_HEALTH_INPUT};
const char* BRAIN_INPUT_TYPE_NAMES[N_BRAIN_INPUT_TYPES] = {
    "Entity (target)", "Distance (target)", "Health (self)"};

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
            default:
                BRAIN_INPUT_TYPE_ERROR("get_brain_input_size", type);
        }
    }

    return input_size;
}

// --------------------------------------------------------
// Brain outputs
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

BrainInputType BRAIN_OUTPUT_TYPES[N_BRAIN_OUTPUT_TYPES] = {
    LOOK_AT_ORIENTATION_OUTPUT,
    MOVE_ORIENTATION_OUTPUT,
    IS_SHOOTING_OUTPUT};
const char* BRAIN_OUTPUT_TYPE_NAMES[N_BRAIN_OUTPUT_TYPES] = {
    "Look at orientation", "Move orientation", "Is shooting"};

BrainOutput init_look_at_orientation_brain_output(void) {
    BrainOutput output;
    output.type = LOOK_AT_ORIENTATION_OUTPUT;

    LookAtOrientationBrainOutput look_at_orientation;
    output.o.look_at_orientation = look_at_orientation;
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

void change_brain_output_type(
    BrainOutput* brain_output, BrainOutputType target_type
) {
    BrainOutputType source_type = brain_output->type;
    if (source_type == target_type) {
        return;
    }

    switch (target_type) {
        case LOOK_AT_ORIENTATION_OUTPUT:
            *brain_output = init_look_at_orientation_brain_output();
            break;
        case MOVE_ORIENTATION_OUTPUT:
            *brain_output = init_move_orientation_brain_output();
            break;
        case IS_SHOOTING_OUTPUT:
            *brain_output = init_is_shooting_brain_output();
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
            case LOOK_AT_ORIENTATION_OUTPUT:
                output_size += n_view_rays;
                break;
            case MOVE_ORIENTATION_OUTPUT:
                output_size += output.o.move_orientation.n_directions;
                break;
            case IS_SHOOTING_OUTPUT:
                output_size += 1;
                break;
            default:
                BRAIN_INPUT_TYPE_ERROR("get_brain_output_size", type);
        }
    }

    return output_size;
}

// --------------------------------------------------------
// Brain general
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

Brain init_empty_brain(void) {
    Brain brain = {0};
    return brain;
}

Brain init_brain(BrainParams params) {
    int n_weights = get_brain_size(params);
    int weights_n_bytes = sizeof(float) * n_weights;
    float* weights = (float*)malloc(weights_n_bytes);

    // for (int i = 0; i < n_weights; ++i) {
    //     weights[i] = i;
    // }

    srand(time(NULL));
    for (int i = 0; i < n_weights; ++i) {
        weights[i] = ((float)rand() / RAND_MAX) * 2.0 - 1.0;
    }

    Brain brain = {.params = params, .weights = weights};
    return brain;
}

void destroy_brain(Brain* brain) {
    if (brain->weights != NULL) {
        free(brain->weights);
    }

    memset(brain, 0, sizeof(Brain));
}

int load_brain(char* file_path, Brain* brain, ResultMessage* res_msg) {
    FILE* fp = open_file(file_path, res_msg, "rb");
    if (res_msg->flag != SUCCESS_RESULT) {
        return 0;
    }

    destroy_brain(brain);

    int version;
    int n_bytes = 0;
    n_bytes += fread(&version, sizeof(int), 1, fp);
    if (version != BRAIN_VERSION) {
        sprintf(
            res_msg->msg,
            "ERROR: Brain version %d is not compatible with the engine, "
            "expecting the version %d\n",
            version,
            BRAIN_VERSION
        );
        return 0;
    }

    n_bytes += fread(&brain->params, sizeof(BrainParams), 1, fp);
    int n_weights = get_brain_size(brain->params);

    brain->weights = (float*)malloc(n_weights * sizeof(float));
    n_bytes += fread(brain->weights, sizeof(float), n_weights, fp);

    fclose(fp);
    res_msg->flag = SUCCESS_RESULT;

    sprintf(res_msg->msg, "INFO: Brain is loaded (%dB)", n_bytes);
    return n_bytes;
}

void save_brain(char* file_path, Brain* brain, ResultMessage* res_msg) {
    FILE* fp = open_file(file_path, res_msg, "wb");
    if (res_msg->flag != SUCCESS_RESULT) {
        return;
    }

    float* weights = brain->weights;
    int n_weights = get_brain_size(brain->params);
    brain->weights = NULL;

    if (weights == NULL || n_weights == 0) {
        strcpy(
            res_msg->msg, "ERROR: Can't save the Brain without weights"
        );
        return;
    }

    int version = BRAIN_VERSION;
    int n_bytes = 0;
    n_bytes += fwrite(&version, sizeof(int), 1, fp);
    n_bytes += fwrite(&brain->params, sizeof(BrainParams), 1, fp);
    n_bytes += fwrite(weights, sizeof(float), n_weights, fp);

    fclose(fp);
    res_msg->flag = SUCCESS_RESULT;

    sprintf(res_msg->msg, "INFO: Brain is saved (%dB)", n_bytes);
    return;
}
