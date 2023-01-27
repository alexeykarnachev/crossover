#include "../app.h"
#include "../component.h"
#include "../debug/debug.h"
#include "../math.h"
#include "../system.h"
#include "../world.h"
#include <math.h>

static Vec2 screen_to_world(Vec2 screen_pos) {
    CameraFrustum frustum = get_camera_frustum();
    Vec2 size = sub(frustum.top_right, frustum.bot_left);
    float x = frustum.bot_left.x + size.x * screen_pos.x;
    float y = frustum.bot_left.y + size.y * screen_pos.y;
    return vec2(x, y);
}

void update_player() {
    if (WORLD.player != -1) {
        Kinematic* kinematic = &WORLD.kinematic[WORLD.player];
        Transformation* transformation
            = &WORLD.transformation[WORLD.player];
        Vec2 velocity = {0.0, 0.0};

        velocity.y += 1.0 * APP.key_states[GLFW_KEY_W];
        velocity.y -= 1.0 * APP.key_states[GLFW_KEY_S];
        velocity.x -= 1.0 * APP.key_states[GLFW_KEY_A];
        velocity.x += 1.0 * APP.key_states[GLFW_KEY_D];
        if (length(velocity) > EPS) {
            velocity = scale(normalize(velocity), kinematic->max_speed);
        }
        kinematic->velocity = velocity;
        Vec2 look_at = screen_to_world(get_cursor_screen_pos());
        kinematic->orientation = atan2(
            look_at.y - transformation->position.y,
            look_at.x - transformation->position.x
        );

        DEBUG.general.look_at = look_at;
        if (DEBUG.shading.look_at) {
            render_debug_circle(look_at, 0.1, RED_COLOR, -1);
        }

        if (APP.mouse_button_states[GLFW_MOUSE_BUTTON_1]) {
            if (entity_has_component(WORLD.player, GUN_COMPONENT)) {
                Gun* gun = &WORLD.gun[WORLD.player];
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
                    float bullet_collider_length = gun->bullet.speed
                                                   / 30.0;
                    Primitive bullet_primitive = init_circle_primitive(0.1
                    );
                    Primitive bullet_collider = init_line_primitive(
                        vec2(bullet_collider_length, 0.0)
                    );
                    Material bullet_material = init_material(RED_COLOR);

                    spawn_bullet(
                        bullet_transformation,
                        bullet_primitive,
                        bullet_collider,
                        bullet_material,
                        bullet_kinematic,
                        gun->bullet.ttl,
                        WORLD.player
                    );
                }
            }
        }
    }
}
