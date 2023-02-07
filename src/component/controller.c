#include "controller.h"

#include <stdio.h>
#include <stdlib.h>

#define CONTROLLER_TYPE_ERROR(fn_name, type) \
    do { \
        fprintf( \
            stderr, \
            "ERROR: can't %s for the controller with type id: %d. Needs " \
            "to be implemented\n", \
            fn_name, \
            type \
        ); \
        exit(1); \
    } while (0)

ControllerType CONTROLLER_TYPES[N_CONTROLLER_TYPES] = {
    PLAYER_KEYBOARD_CONTROLLER, DUMMY_AI_CONTROLLER, BRAIN_AI_CONTROLLER};
const char* CONTROLLER_TYPE_NAMES[N_CONTROLLER_TYPES] = {
    "Player keyboard", "Dummy AI", "Brain AI"};

void change_controller_type(
    Controller* controller, ControllerType target_type
) {
    ControllerType source_type = controller->type;
    if (source_type == target_type) {
        return;
    }

    switch (target_type) {
        case PLAYER_KEYBOARD_CONTROLLER:
            *controller = init_player_keyboard_controller();
            break;
        case DUMMY_AI_CONTROLLER:
            *controller = init_dummy_ai_controller();
            break;
        case BRAIN_AI_CONTROLLER:
            *controller = init_brain_ai_controller();
            break;
        default:
            CONTROLLER_TYPE_ERROR("change_controller_type", source_type);
    }
}

Controller init_player_keyboard_controller() {
    Controller controller = {0};

    controller.type = PLAYER_KEYBOARD_CONTROLLER;
    PlayerKeyboardController player_keyboard_controller;
    controller.c.player_keyboard = player_keyboard_controller;
    return controller;
}

Controller init_dummy_ai_controller() {
    Controller controller = {0};

    controller.type = DUMMY_AI_CONTROLLER;
    DummyAIController dummy_ai_controller = {0};
    controller.c.dummy_ai = dummy_ai_controller;
    return controller;
}

Controller init_brain_ai_controller() {
    Controller controller = {0};

    controller.type = BRAIN_AI_CONTROLLER;
    BrainAIController brain_ai_controller;
    controller.c.brain_ai = brain_ai_controller;
    return controller;
}
