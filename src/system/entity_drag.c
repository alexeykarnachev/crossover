#include "../app.h"
#include "../debug.h"
#include "../math.h"
#include "../system.h"
#include "../world.h"

void update_entity_drag() {
    int entity = DEBUG.picked_entity;
    if (entity == -1 || !APP.mouse_button_states[GLFW_MOUSE_BUTTON_1]
        || !entity_has_component(entity, TRANSFORMATION_COMPONENT)) {

        DEBUG.is_dragging = 0;
        return;
    }

    Transformation* transformation = &WORLD.transformations[entity];
    Vec2 cursor_world_pos = get_cursor_world_pos();
    if (dist(cursor_world_pos, transformation->position) <= 0.1) {
        DEBUG.is_dragging = 1;
    }

    if (DEBUG.is_dragging) {
        transformation->position = cursor_world_pos;
    }
}
