#include "brain.h"

#include "../scene.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

BrainInputType BRAIN_INPUT_TYPES[N_BRAIN_INPUT_TYPES] = {
    IS_OBSTACLE_INPUT, IS_ENEMY_INPUT, DISTANCE_INPUT};
BrainInputType BRAIN_OUTPUT_TYPES[N_BRAIN_OUTPUT_TYPES] = {
    MOVE_DIRECTION_OUTPUT, IS_SHOOT_OUTPUT};

static uint64_t VISION_INPUT_TYPES = IS_OBSTACLE_INPUT | IS_ENEMY_INPUT
                                     | DISTANCE_INPUT;

static int get_brain_input_size(int entity, uint64_t input_types) {
    int has_vision = check_if_entity_has_component(
        entity, VISION_COMPONENT
    );
    if ((input_types & VISION_INPUT_TYPES) && !has_vision) {
        fprintf(
            stderr,
            "ERROR: Can't initialize brain. One of the input features "
            "requires Vision component, but the entity doesn't have it\n"
        );
        exit(1);
    }

    int input_size = 0;
    Vision vision = SCENE.visions[entity];
    for (int i = 0; i < N_BRAIN_INPUT_TYPES; ++i) {
        BrainInputType type = BRAIN_INPUT_TYPES[i];
        if (type & input_types == 0) {
            continue;
        } else if (type & VISION_INPUT_TYPES) {
            input_size += vision.n_view_rays;
        }
    }

    return input_size;
}

static int get_brain_output_size(int entity, uint64_t output_types) {
    int output_size = 0;

    for (int i = 0; i < N_BRAIN_OUTPUT_TYPES; ++i) {
        BrainOutputType type = BRAIN_OUTPUT_TYPES[i];
        if (output_types & type == 0) {
            continue;
        } else if (type & MOVE_DIRECTION_OUTPUT) {
            output_size += N_MOVE_DIRECTIONS;
        } else if (type & IS_SHOOT_OUTPUT) {
            output_size += 1;
        }
    }

    return output_size;
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

Brain init_brain(
    int entity,
    int layer_sizes[MAX_N_BRAIN_LAYERS],
    int n_layers,
    uint64_t input_types,
    uint64_t output_types
) {
    Brain brain = {0};
    memcpy(brain.layer_sizes, layer_sizes, sizeof(int) * n_layers);
    brain.n_layers = n_layers;
    brain.input_size = get_brain_input_size(entity, input_types);
    brain.output_size = get_brain_output_size(entity, output_types);
    brain.weights = init_brain_weights(
        layer_sizes, n_layers, brain.input_size, brain.output_size
    );
}
