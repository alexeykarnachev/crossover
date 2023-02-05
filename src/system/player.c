#include "../app.h"
#include "../component.h"
#include "../debug.h"
#include "../gl.h"
#include "../math.h"
#include "../scene.h"
#include "../system.h"
#include <math.h>

void update_player() {
    if (SCENE.player == -1 || SCENE.camera == -1) {
        return;
    }
    Kinematic* kinematic = &SCENE.kinematics[SCENE.player];
    Transformation* transformation = &SCENE.transformations[SCENE.player];
    Transformation camera = SCENE.transformations[SCENE.camera];

    Vec2 look_at = get_cursor_scene_pos();
    DEBUG.general.look_at = look_at;

    Vec2 velocity = {0.0, 0.0};
    velocity.y += 1.0 * APP.key_states[GLFW_KEY_W];
    velocity.y -= 1.0 * APP.key_states[GLFW_KEY_S];
    velocity.x -= 1.0 * APP.key_states[GLFW_KEY_A];
    velocity.x += 1.0 * APP.key_states[GLFW_KEY_D];
    velocity = rotate(velocity, vec2(0.0, 0.0), camera.orientation);
    kinematic->orientation = atan2(
        look_at.y - transformation->position.y,
        look_at.x - transformation->position.x
    );
    if (length(velocity) > EPS) {
        velocity = scale(normalize(velocity), kinematic->max_speed);
    }
    kinematic->velocity = velocity;

    if (APP.mouse_button_states[GLFW_MOUSE_BUTTON_1]) {
        if (check_if_entity_has_component(SCENE.player, GUN_COMPONENT)) {
            Gun* gun = &SCENE.guns[SCENE.player];
            float time_since_last_shoot
                = (APP.time - gun->last_time_shoot);
            if (gun->last_time_shoot == 0
                || (time_since_last_shoot > 1.0 / gun->fire_rate)) {
                gun->last_time_shoot = APP.time;
                Vec2 bullet_velocity = vec2(
                    cos(transformation->orientation),
                    sin(transformation->orientation)
                );
                bullet_velocity = scale(
                    bullet_velocity, gun->bullet.speed
                );
                Kinematic bullet_kinematic = {
                    bullet_velocity,
                    gun->bullet.speed,
                    transformation->orientation,
                    0.0};
                Transformation bullet_transformation = *transformation;

                spawn_kinematic_bullet(
                    bullet_transformation,
                    bullet_kinematic,
                    gun->bullet.ttl,
                    SCENE.player
                );
            }
        }
    }
}
