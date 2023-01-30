#include "../app.h"
#include "../const.h"
#include "../debug.h"
#include "../math.h"
#include "../system.h"
#include "../world.h"

static Vec2 PREV_CURSOR_WORLD_POS;

void update_entity_drag() {
    int entity = DEBUG.picked_entity;
    Vec2 cursor_world_pos = get_cursor_world_pos();

    if (entity == -1 || !APP.mouse_button_states[GLFW_MOUSE_BUTTON_1]
        || !entity_has_component(entity, TRANSFORMATION_COMPONENT)) {

        DEBUG.is_dragging = 0;
    } else {
        Transformation* transformation = &WORLD.transformations[entity];
        float drag_handle_size = WORLD.camera_view_width
                                 * DRAG_HANDLE_SCALE;
        if (dist(cursor_world_pos, transformation->position)
            <= drag_handle_size) {
            DEBUG.is_dragging = 1;
        }

        if (DEBUG.is_dragging) {
            Vec2 diff = sub(cursor_world_pos, PREV_CURSOR_WORLD_POS);
            transformation->position = add(transformation->position, diff);
        }
    }

    PREV_CURSOR_WORLD_POS = cursor_world_pos;
}
