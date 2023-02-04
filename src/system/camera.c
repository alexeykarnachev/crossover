#include "../app.h"
#include "../component.h"
#include "../const.h"
#include "../debug.h"
#include "../scene.h"
#include "../system.h"

static Vec2 PREV_CURSOR_SCENE_POS;

void update_camera() {
    if (SCENE.camera == -1) {
        return;
    }

    Transformation* camera = &SCENE.transformations[SCENE.camera];

    SCENE.camera_view_width -= CAMERA_SCROLL_SENSITIVITY * APP.scroll_dy;
    SCENE.camera_view_width = max(EPS, SCENE.camera_view_width);

    if (APP.mouse_button_states[GLFW_MOUSE_BUTTON_MIDDLE]) {
        Vec2 cursor_scene_pos = get_cursor_scene_pos();
        Vec2 diff = sub(cursor_scene_pos, PREV_CURSOR_SCENE_POS);
        diff = rotate(diff, vec2(0.0, 0.0), -camera->orientation);
        camera->position = sub(camera->position, diff);
    }

    DEBUG.general.camera_position = camera->position;
    PREV_CURSOR_SCENE_POS = get_cursor_scene_pos();
}
