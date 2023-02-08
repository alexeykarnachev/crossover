#include "brain.h"

#include "../scene.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    TARGET_ENTITY_INPUT,
    TARGET_DISTANCE_INPUT,
    TARGET_HEALTH_INPUT,
    SELF_HEALTH_INPUT};
const char* BRAIN_INPUT_TYPE_NAMES[N_BRAIN_INPUT_TYPES] = {
    "Entity (target)",
    "Distance (target)",
    "Health (target)",
    "Health (self)"};

BrainInputType BRAIN_OUTPUT_TYPES[N_BRAIN_OUTPUT_TYPES] = {
    MOVE_DIRECTION_OUTPUT, IS_SHOOT_OUTPUT};

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

BrainInput init_target_health_brain_input(void) {
    BrainInput input;
    input.type = TARGET_HEALTH_INPUT;

    TargetHealthBrainInput target_health;
    input.i.target_health = target_health;
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
        case TARGET_HEALTH_INPUT:
            *brain_input = init_target_health_brain_input();
            break;
        case SELF_HEALTH_INPUT:
            *brain_input = init_self_health_brain_input();
            break;
        default:
            BRAIN_INPUT_TYPE_ERROR("change_brain_input_type", source_type);
    }
}

static int get_brain_input_size(int entity, uint64_t input_types) {
    return 69;
    // int has_vision = check_if_entity_has_component(
    //     entity, VISION_COMPONENT
    // );
    // if ((input_types & VISION_INPUT_TYPES) && !has_vision) {
    //     fprintf(
    //         stderr,
    //         "ERROR: Can't initialize brain. One of the input features "
    //         "requires Vision component, but the entity doesn't have
    //         it\n"
    //     );
    //     exit(1);
    // }

    // int input_size = 0;
    // Vision vision = SCENE.visions[entity];
    // for (int i = 0; i < N_BRAIN_INPUT_TYPES; ++i) {
    //     BrainInputType type = BRAIN_INPUT_TYPES[i];
    //     if (type & input_types == 0) {
    //         continue;
    //     } else if (type & VISION_INPUT_TYPES) {
    //         input_size += vision.n_view_rays;
    //     }
    // }

    // return input_size;
}

static int get_brain_output_size(int entity, uint64_t output_types) {
    return 228;
    // int output_size = 0;

    // for (int i = 0; i < N_BRAIN_OUTPUT_TYPES; ++i) {
    //     BrainOutputType type = BRAIN_OUTPUT_TYPES[i];
    //     if (output_types & type == 0) {
    //         continue;
    //     } else if (type & MOVE_DIRECTION_OUTPUT) {
    //         output_size += N_MOVE_DIRECTIONS;
    //     } else if (type & IS_SHOOT_OUTPUT) {
    //         output_size += 1;
    //     }
    // }

    // return output_size;
}

static float* init_brain_weights(
    int layer_sizes[MAX_N_BRAIN_LAYERS],
    int n_layers,
    int input_size,
    int output_size
) {
    if (!input_size || !output_size) {
        fprintf(
            stderr,
            "ERROR: Can't initialize Brain weights. Input and output "
            "sizes must be positive integers\n"
        );
        exit(1);
    }

    int n_weights = 0;
    int inp_layer_size = input_size;
    int out_layer_size;
    for (int i = 0; i < n_layers + 1; ++i) {
        out_layer_size = i >= n_layers ? output_size : layer_sizes[i];
        n_weights += (inp_layer_size + 1) * out_layer_size;
        inp_layer_size = out_layer_size;
    }

    float* weights = (float*)malloc(sizeof(float) * n_weights);
    return weights;
}

// Brain init_brain(
//     int entity,
//     int layer_sizes[MAX_N_BRAIN_LAYERS],
//     int n_layers,
//     uint64_t input_types,
//     uint64_t output_types
// ) {
//     Brain brain = {0};
//     memcpy(brain.layer_sizes, layer_sizes, sizeof(int) * n_layers);
//     brain.n_layers = n_layers;
//     brain.input_size = get_brain_input_size(entity, input_types);
//     brain.output_size = get_brain_output_size(entity, output_types);
//     brain.weights = init_brain_weights(
//         layer_sizes, n_layers, brain.input_size, brain.output_size
//     );
// }
