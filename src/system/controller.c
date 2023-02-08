#include "../app.h"
#include "../component.h"
#include "../debug.h"
#include "../gl.h"
#include "../math.h"
#include "../scene.h"
#include "../system.h"
#include <math.h>
#include <stdlib.h>

static void try_shoot(int entity) {
    if (!check_if_entity_has_component(entity, GUN_COMPONENT)) {
        return;
    }

    Gun* gun = &SCENE.guns[entity];
    Transformation transformation = SCENE.transformations[entity];

    float time_since_last_shoot = (SCENE.time - gun->last_time_shoot);
    float shoot_period = 1.0 / gun->fire_rate;
    int can_shoot = gun->last_time_shoot == 0
                    || time_since_last_shoot > shoot_period;
    if (can_shoot) {
        gun->last_time_shoot = SCENE.time;
        KinematicMovement movement = init_kinematic_movement(
            gun->bullet.speed,
            transformation.orientation,
            transformation.orientation,
            1
        );
        spawn_bullet(transformation, movement, gun->bullet.ttl, entity);
    }
}

typedef struct ControllerAction {
    float watch_orientation;
    float move_orientation;
    int is_shooting;
    int is_moving;
} ControllerAction;

static ControllerAction get_player_keyboard_action(int entity) {
    int required_component = TRANSFORMATION_COMPONENT
                             | KINEMATIC_MOVEMENT_COMPONENT;
    if (!check_if_entity_has_component(entity, required_component)) {
        fprintf(
            stderr,
            "ERROR: Can't get the player keyboard action. The entity "
            "doesn't have Transformation or KinematicMovement component\n"
        );
        exit(1);
    }
    if (SCENE.camera == -1) {
        fprintf(
            stderr,
            "ERROR: Can't get the player keyboard action. The Scene "
            "doesn't have the Camera entity\n"
        );
        exit(1);
    }

    ControllerAction action = {0};
    Transformation* transformation = &SCENE.transformations[entity];

    Vec2 look_at = get_cursor_scene_pos();
    action.watch_orientation = get_vec_orientation(
        sub(look_at, transformation->position)
    );

    Vec2 move_dir = {0.0, 0.0};
    move_dir.y += 1.0 * APP.key_states[GLFW_KEY_W];
    move_dir.y -= 1.0 * APP.key_states[GLFW_KEY_S];
    move_dir.x -= 1.0 * APP.key_states[GLFW_KEY_A];
    move_dir.x += 1.0 * APP.key_states[GLFW_KEY_D];

    if (length(move_dir) > EPS) {
        Transformation camera = SCENE.transformations[SCENE.camera];
        move_dir = rotate(move_dir, vec2(0.0, 0.0), camera.orientation);
        action.move_orientation = get_vec_orientation(move_dir);
        action.is_moving = 1;
    }

    action.is_shooting = APP.mouse_button_states[GLFW_MOUSE_BUTTON_1];

    return action;
}

static ControllerAction get_dummy_ai_action(int entity) {
    int required_component = TRANSFORMATION_COMPONENT | VISION_COMPONENT;
    if (!check_if_entity_has_component(entity, required_component)) {
        fprintf(
            stderr,
            "ERROR: Can't get the dummy ai controller action. The entity "
            "doesn't have Transformation or Vision component\n"
        );
        exit(1);
    }

    ControllerAction action = {0};
    DummyAIController ai = SCENE.controllers[entity].c.dummy_ai;
    Vision vision = SCENE.visions[entity];
    Vec2 position = SCENE.transformations[entity].position;
    action.is_moving = 0;

    Vec2 nearest_target_position;
    float nearest_dist = HUGE_VAL;
    for (int i = 0; i < vision.n_view_rays; ++i) {
        RayCastResult observation = vision.observations[i];
        int target = observation.entity;

        if (target != -1
            && check_if_entity_has_component(
                target, CONTROLLER_COMPONENT
            )) {
            Vec2 target_position = observation.position;
            float d = dist(position, target_position);
            if (d < nearest_dist) {
                nearest_dist = d;
                nearest_target_position = target_position;
            }
        }
    }

    if (nearest_dist < HUGE_VAL) {
        action.is_moving = 1;
        action.move_orientation = get_vec_orientation(
            sub(nearest_target_position, position)
        );
        action.watch_orientation = action.move_orientation;
        action.is_shooting = ai.is_shooting;
    }

    return action;
}

static ControllerAction get_brain_ai_action(int entity) {
    int required_component = TRANSFORMATION_COMPONENT | VISION_COMPONENT;
    if (!check_if_entity_has_component(entity, required_component)) {
        fprintf(
            stderr,
            "ERROR: Can't get the brain ai controller action. The entity "
            "doesn't have Transformation or Vision component\n"
        );
        exit(1);
    }

    ControllerAction action = {0};
    BrainAIController ai = SCENE.controllers[entity].c.brain_ai;
    Brain brain = ai.brain;

    return action;
}

void update_controllers() {
    for (int entity = 0; entity < SCENE.n_entities; ++entity) {
        int required_component = TRANSFORMATION_COMPONENT
                                 | KINEMATIC_MOVEMENT_COMPONENT
                                 | CONTROLLER_COMPONENT;
        if (!check_if_entity_has_component(entity, required_component)) {
            continue;
        }

        ControllerType type = SCENE.controllers[entity].type;
        ControllerAction action;
        switch (type) {
            case PLAYER_KEYBOARD_CONTROLLER: {
                action = get_player_keyboard_action(entity);
                break;
            }
            case DUMMY_AI_CONTROLLER: {
                action = get_dummy_ai_action(entity);
                break;
            }
            case BRAIN_AI_CONTROLLER: {
                action = get_brain_ai_action(entity);
                break;
            }
        }

        KinematicMovement* movement = &SCENE.kinematic_movements[entity];
        movement->watch_orientation = action.watch_orientation;
        movement->move_orientation = action.move_orientation;
        movement->is_moving = action.is_moving;
        if (action.is_shooting) {
            try_shoot(entity);
        }
    }
}
