#include "../app.h"
#include "../component.h"
#include "../debug.h"
#include "../gl.h"
#include "../system.h"
#include "../world.h"

Vec2 get_cursor_world_pos(void) {
    Vec2 screen_pos = get_cursor_screen_pos();
    CameraFrustum frustum = get_camera_frustum();
    Vec2 size = sub(frustum.top_right, frustum.bot_left);
    float x = frustum.bot_left.x + size.x * screen_pos.x;
    float y = frustum.bot_left.y + size.y * screen_pos.y;
    return vec2(x, y);
}

void update_cursor_picking(void) {
    if (!APP.mouse_button_states[GLFW_MOUSE_BUTTON_1]) {
        return;
    }

    DEBUG.picked_entity = -1;
    Vec2 cursor_world_pos = get_cursor_world_pos();
    Primitive cursor_primitive = init_circle_primitive(0.1);
    Transformation cursor_transformation = init_transformation(
        cursor_world_pos, 0.0
    );
    for (int entity = 0; entity < WORLD.n_entities; ++entity) {
        if (!entity_can_be_rendered(entity)) {
            continue;
        }

        Primitive entity_primitive = WORLD.primitives[entity];
        Transformation entity_transformation
            = WORLD.transformations[entity];
        Collision collision;
        int is_picked = collide_primitives(
            cursor_primitive,
            cursor_transformation,
            entity_primitive,
            entity_transformation,
            &collision
        );

        if (is_picked) {
            DEBUG.picked_entity = entity;
            break;
        }
    }
}

void render_cursor_picking(void) {
    if (DEBUG.picked_entity == -1) {
        return;
    }

    Primitive primitive = WORLD.primitives[DEBUG.picked_entity];
    Transformation transformation
        = WORLD.transformations[DEBUG.picked_entity];
    Rectangle rectangle = get_primitive_bounding_rectangle(
        primitive, transformation
    );
    render_debug_rectangle(
        transformation.position,
        rectangle.width,
        rectangle.height,
        YELLOW_COLOR,
        LINE
    );
}
