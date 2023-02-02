#include "../app.h"
#include "../component.h"
#include "../debug.h"
#include "../math.h"
#include "../system.h"
#include "../world.h"
#include <math.h>

void update_player() {
    if (WORLD.player == -1 || WORLD.camera == -1) {
        return;
    }
    Kinematic* kinematic = &WORLD.kinematics[WORLD.player];
    Transformation* transformation = &WORLD.transformations[WORLD.player];
    Transformation camera = WORLD.transformations[WORLD.camera];

    Vec2 look_at = get_cursor_world_pos();
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
        if (entity_has_component(WORLD.player, GUN_COMPONENT)) {
            Gun* gun = &WORLD.guns[WORLD.player];
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

                spawn_bullet(
                    bullet_transformation,
                    bullet_kinematic,
                    gun->bullet.ttl,
                    WORLD.player
                );
            }
        }
    }
}

void render_debug_player() {
    if (WORLD.player == -1) {
        return;
    }
    if (DEBUG.is_playing) {
        render_debug_circle(DEBUG.general.look_at, 0.1, RED_COLOR, -1);
    }
}
