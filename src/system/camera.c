#include "../app.h"
#include "../component.h"
#include "../const.h"
#include "../debug.h"
#include "../system.h"
#include "../world.h"

void update_camera() {
    if (WORLD.camera == -1) {
        return;
    }

    Transformation camera = WORLD.transformations[WORLD.camera];
    DEBUG.general.camera_position = camera.position;

    WORLD.camera_view_width -= CAMERA_SCROLL_SENSITIVITY * APP.scroll_dy;
    WORLD.camera_view_width = max(EPS, WORLD.camera_view_width);
}
