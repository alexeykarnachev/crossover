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
        Vec2 velocity = get_orientation_vec(transformation.orientation);
        velocity = scale(velocity, gun->bullet.speed);
        Kinematic kinematic = {
            velocity, gun->bullet.speed, transformation.orientation, 0.0};

        spawn_bullet(transformation, kinematic, gun->bullet.ttl, entity);
    }
}

static void update_player_keyboard_controller(int entity) {
    if (SCENE.camera == -1) {
        return;
    }

    int components = TRANSFORMATION_COMPONENT | KINEMATIC_COMPONENT;
    if (!check_if_entity_has_component(entity, components)) {
        return;
    }

    Kinematic* kinematic = &SCENE.kinematics[entity];
    Transformation* transformation = &SCENE.transformations[entity];
    Transformation camera = SCENE.transformations[SCENE.camera];

    Vec2 look_at = get_cursor_scene_pos();

    Vec2 move_dir = {0.0, 0.0};
    move_dir.y += 1.0 * APP.key_states[GLFW_KEY_W];
    move_dir.y -= 1.0 * APP.key_states[GLFW_KEY_S];
    move_dir.x -= 1.0 * APP.key_states[GLFW_KEY_A];
    move_dir.x += 1.0 * APP.key_states[GLFW_KEY_D];
    move_dir = rotate(move_dir, vec2(0.0, 0.0), camera.orientation);

    kinematic->orientation = get_vec_orientation(
        sub(look_at, transformation->position)
    );
    Vec2 velocity = {0.0};
    if (length(move_dir) > EPS) {
        move_dir = normalize(move_dir);
        velocity = scale(move_dir, kinematic->max_speed);
    }
    kinematic->velocity = velocity;

    if (APP.mouse_button_states[GLFW_MOUSE_BUTTON_1]) {
        try_shoot(entity);
    }
}

static void update_dummy_ai_controller(int entity) {}

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
        }
    }
}
