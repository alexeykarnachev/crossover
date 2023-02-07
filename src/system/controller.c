#include "../app.h"
#include "../component.h"
#include "../debug.h"
#include "../gl.h"
#include "../math.h"
#include "../scene.h"
#include "../system.h"
#include <math.h>

static void try_shoot(int entity) {
    if (!check_if_entity_has_component(entity, GUN_COMPONENT)) {
        return;
    }

    Gun* gun = &SCENE.guns[entity];
    Transformation transformation = SCENE.transformations[entity];

    float time_since_last_shoot = (APP.time - gun->last_time_shoot);
    float shoot_period = 1.0 / gun->fire_rate;
    int can_shoot = gun->last_time_shoot == 0
                    || time_since_last_shoot > shoot_period;
    if (can_shoot) {
        gun->last_time_shoot = APP.time;
        Vec2 move_dir = get_orientation_vec(transformation.orientation);
        KinematicMovement movement = init_kinematic_movement(
            move_dir, gun->bullet.speed, transformation.orientation, 1
        );
        spawn_bullet(transformation, movement, gun->bullet.ttl, entity);
    }
}

static void update_player_keyboard_controller(int entity) {
    int required_component = TRANSFORMATION_COMPONENT
                             | KINEMATIC_MOVEMENT_COMPONENT;
    if (!check_if_entity_has_component(entity, required_component)
        || SCENE.camera == -1) {
        return;
    }

    KinematicMovement* movement = &SCENE.kinematic_movements[entity];
    Transformation* transformation = &SCENE.transformations[entity];
    Transformation camera = SCENE.transformations[SCENE.camera];

    Vec2 look_at = get_cursor_scene_pos();
    movement->target_orientation = get_vec_orientation(
        sub(look_at, transformation->position)
    );

    Vec2 move_dir = {0.0, 0.0};
    move_dir.y += 1.0 * APP.key_states[GLFW_KEY_W];
    move_dir.y -= 1.0 * APP.key_states[GLFW_KEY_S];
    move_dir.x -= 1.0 * APP.key_states[GLFW_KEY_A];
    move_dir.x += 1.0 * APP.key_states[GLFW_KEY_D];
    move_dir = rotate(move_dir, vec2(0.0, 0.0), camera.orientation);
    movement->move_dir = move_dir;

    if (APP.mouse_button_states[GLFW_MOUSE_BUTTON_1]) {
        try_shoot(entity);
    }
}

static void update_dummy_ai_controller(int entity) {
    int required_component = TRANSFORMATION_COMPONENT
                             | KINEMATIC_MOVEMENT_COMPONENT
                             | VISION_COMPONENT;
    if (!check_if_entity_has_component(entity, required_component)) {
        return;
    }

    DummyAIController dummy_ai = SCENE.controllers[entity].c.dummy_ai;
    Vision vision = SCENE.visions[entity];
    Vec2 position = SCENE.transformations[entity].position;
    KinematicMovement* movement = &SCENE.kinematic_movements[entity];
    movement->move_dir = vec2(0.0, 0.0);

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
        movement->move_dir = sub(nearest_target_position, position);
        movement->target_orientation = get_vec_orientation(
            movement->move_dir
        );

        if (dummy_ai.is_shooting) {
            try_shoot(entity);
        }
    }
}

static void update_brain_ai_controller(int entity) {
    int required_component = TRANSFORMATION_COMPONENT
                             | KINEMATIC_MOVEMENT_COMPONENT
                             | VISION_COMPONENT;
    if (!check_if_entity_has_component(entity, required_component)) {
        return;
    }
    Vision vision = SCENE.visions[entity];
    Vec2 position = SCENE.transformations[entity].position;
    Controller controller = SCENE.controllers[entity];
    KinematicMovement* movement = &SCENE.kinematic_movements[entity];
}

void update_controllers() {
    for (int entity = 0; entity < SCENE.n_entities; ++entity) {
        if (!check_if_entity_has_component(entity, CONTROLLER_COMPONENT)) {
            continue;
        }

        ControllerType type = SCENE.controllers[entity].type;
        switch (type) {
            case PLAYER_KEYBOARD_CONTROLLER: {
                update_player_keyboard_controller(entity);
                break;
            }
            case DUMMY_AI_CONTROLLER: {
                update_dummy_ai_controller(entity);
                break;
            }
                // case BRAIN_AI_CONTROLLER: {
                //     update_brain_ai_controller(entity);
                //     break;
                // }
        }
    }
}
