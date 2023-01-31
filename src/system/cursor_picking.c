#include "../app.h"
#include "../component.h"
#include "../const.h"
#include "../debug.h"
#include "../gl.h"
#include "../math.h"
#include "../system.h"
#include "../world.h"

Vec2 get_cursor_world_pos(void) {
    Vec2 screen_pos = get_cursor_screen_pos();
    CameraFrustum frustum = get_camera_frustum();
    Vec2 size = sub(frustum.top_right, frustum.bot_left);
    float x = frustum.bot_left.x + size.x * screen_pos.x;
    float y = frustum.bot_left.y + size.y * screen_pos.y;

    Transformation camera = WORLD.transformations[WORLD.camera];
    Vec2 position = rotate(vec2(x, y), vec2(0.0, 0.0), camera.orientation);
    return position;
}

static int check_if_picked(int entity) {
    if (!entity_can_be_rendered(entity)) {
        return 0;
    }

    Vec2 cursor_world_pos = get_cursor_world_pos();
    Primitive cursor_primitive = init_circle_primitive(0.1);
    Transformation cursor_transformation = init_transformation(
        cursor_world_pos, 0.0
    );
    Primitive entity_primitive = WORLD.primitives[entity];
    Transformation entity_transformation = WORLD.transformations[entity];
    Collision collision;
    int is_picked = collide_primitives(
        cursor_primitive,
        cursor_transformation,
        entity_primitive,
        entity_transformation,
        &collision
    );
    return is_picked;
}

void update_cursor_picking(void) {
    // Don't update picking if mouse is not pressed or
    // the entity dragging mode is on
    if (!APP.mouse_button_states[GLFW_MOUSE_BUTTON_1]
        || DEBUG.is_dragging) {
        return;
    }

    // Don't even try to pick another entities, if the current picked
    // entity could be picked again
    if (DEBUG.picked_entity != -1) {
        if (check_if_picked(DEBUG.picked_entity)) {
            return;
        }
    }

    // Try to pick another entities
    DEBUG.picked_entity = -1;
    for (int entity = 0; entity < WORLD.n_entities; ++entity) {
        if (check_if_picked(entity)) {
            DEBUG.picked_entity = entity;
            break;
        }
    }
}

void render_cursor_picking(void) {
    if (DEBUG.picked_entity == -1
        || entity_can_be_rendered(!DEBUG.picked_entity)) {
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
    float drag_handle_size = WORLD.camera_view_width * DRAG_HANDLE_SCALE;
    render_debug_circle(
        transformation.position, drag_handle_size, YELLOW_COLOR, FILL
    );
}
