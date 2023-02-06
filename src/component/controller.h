#pragma once

#define N_CONTROLLER_TYPES 2
typedef enum ControllerType {
    PLAYER_KEYBOARD_CONTROLLER = 1 << 0,
    DUMMY_AI_CONTROLLER = 1 << 1
} ControllerType;

extern ControllerType CONTROLLER_TYPES[N_CONTROLLER_TYPES];

typedef struct PlayerKeyboardController {
} PlayerKeyboardController;

typedef struct DummyAIController {
} DummyAIController;

typedef struct Controller {
    ControllerType type;
    union {
        PlayerKeyboardController player_keyboard;
        DummyAIController dummy_ai;
    } c;
} Controller;

const char* get_controller_type_name(ControllerType type);
void change_controller_type(
    Controller* controller, ControllerType target_type
);
Controller init_dummy_ai_controller();
Controller init_player_keyboard_controller();
