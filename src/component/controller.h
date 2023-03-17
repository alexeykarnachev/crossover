#pragma once
#include "../asset.h"
#include "../const.h"
#include "../math.h"
#include <stdint.h>

#define N_CONTROLLER_TYPES 3
typedef enum ControllerType {
    PLAYER_KEYBOARD_CONTROLLER,
    DUMMY_AI_CONTROLLER,
    BRAIN_AI_CONTROLLER
} ControllerType;

extern ControllerType CONTROLLER_TYPES[N_CONTROLLER_TYPES];
extern const char* CONTROLLER_TYPE_NAMES[N_CONTROLLER_TYPES];

typedef struct PlayerKeyboardController {
} PlayerKeyboardController;

typedef struct DummyAIController {
    int is_shooting;
} DummyAIController;

typedef struct BrainAIController {
    char key[MAX_PATH_LENGTH + 32];
    float temperature;
} BrainAIController;

typedef struct Controller {
    float kinematic_speed;
    float dynamic_force_magnitude;
    ControllerType type;
    union {
        PlayerKeyboardController player_keyboard;
        DummyAIController dummy_ai;
        BrainAIController brain_ai;
    } c;
} Controller;

void change_controller_type(
    Controller* controller, ControllerType target_type
);
Controller init_player_keyboard_controller(
    float kinematic_speed, float dynamic_force_magnitude
);
Controller init_dummy_ai_controller(
    float kinematic_speed, float dynamic_force_magnitude
);
Controller init_brain_ai_controller(
    float kinematic_speed, float dynamic_force_magnitude, float temperature
);
Controller init_default_player_keyboard_controller(void);
Controller init_default_dummy_ai_controller(void);
Controller init_default_brain_ai_controller(void);
