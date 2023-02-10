#include "brain.h"

#include "component.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Brain BRAINS[MAX_N_BRAINS] = {0};
int N_BRAINS = 0;

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

int get_brain_input_size(Brain brain) {
    int n_view_rays = brain.n_view_rays;
    int input_size = 0;
    for (int i = 0; i < brain.n_inputs; ++i) {
        BrainInput input = brain.inputs[i];
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

int get_brain_output_size(Brain brain) {
    int n_view_rays = brain.n_view_rays;
    int output_size = 0;
    for (int i = 0; i < brain.n_outputs; ++i) {
        BrainOutput output = brain.outputs[i];
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
uint64_t get_brain_required_input_types(Brain brain) {
    uint64_t components = 0;
    for (int i = 0; i < brain.n_inputs; ++i) {
        BrainInputType type = brain.inputs[i].type;
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

int get_brain_size(Brain brain) {
    int input_size = get_brain_input_size(brain);
    int output_size = get_brain_output_size(brain);
    if (!input_size || !output_size) {
        return 0;
    }

    int n_layers = brain.n_layers;
    int* layer_sizes = brain.layer_sizes;
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

void init_brain_weights(Brain* brain) {
    int n_weights = get_brain_size(*brain);
    float* weights = (float*)malloc(sizeof(float) * n_weights);
    brain->weights = weights;
}
