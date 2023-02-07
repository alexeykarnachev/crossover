#pragma once
#include "../const.h"
#include "../math.h"

#define N_CONTROLLER_TYPES 3
typedef enum ControllerType {
    PLAYER_KEYBOARD_CONTROLLER = 1 << 0,
    DUMMY_AI_CONTROLLER = 1 << 1,
    BRAIN_AI_CONTROLLER = 1 << 2
} ControllerType;

extern ControllerType CONTROLLER_TYPES[N_CONTROLLER_TYPES];

typedef struct PlayerKeyboardController {
} PlayerKeyboardController;

typedef struct DummyAIController {
    int is_shooting;
} DummyAIController;

typedef struct Brain {
    Vec2 layer_sizes[MAX_N_BRAIN_LAYERS];
    float* weights;
    int n_layers;
} Brain;

typedef struct BrainAIController {
    Brain brain;
} BrainAIController;

typedef struct Controller {
    ControllerType type;
    union {
        PlayerKeyboardController player_keyboard;
        DummyAIController dummy_ai;
        BrainAIController brain_ai;
    } c;
} Controller;

const char* get_controller_type_name(ControllerType type);
void change_controller_type(
    Controller* controller, ControllerType target_type
);
Controller init_player_keyboard_controller();
Controller init_dummy_ai_controller();
Controller init_brain_ai_controller();
