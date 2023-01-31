#include "../app.h"
#include "../component.h"
#include "../const.h"
#include "../debug.h"
#include "../system.h"
#include "../world.h"

static Vec2 PREV_CURSOR_WORLD_POS;

void update_camera() {
    if (WORLD.camera == -1) {
        return;
    }

    Transformation* camera = &WORLD.transformations[WORLD.camera];

    WORLD.camera_view_width -= CAMERA_SCROLL_SENSITIVITY * APP.scroll_dy;
    WORLD.camera_view_width = max(EPS, WORLD.camera_view_width);

    if (APP.mouse_button_states[GLFW_MOUSE_BUTTON_MIDDLE]) {
        Vec2 cursor_world_pos = get_cursor_world_pos();
        Vec2 diff = sub(cursor_world_pos, PREV_CURSOR_WORLD_POS);
        diff = rotate(diff, vec2(0.0, 0.0), -camera->orientation);
        camera->position = sub(camera->position, diff);
    }

    DEBUG.general.camera_position = camera->position;
    PREV_CURSOR_WORLD_POS = get_cursor_world_pos();
}
