#include "../component.h"
#include "../debug.h"
#include "../system.h"
#include "../world.h"

void update_camera() {
    if (WORLD.camera != -1) {
        Transformation t = WORLD.transformation[WORLD.camera];
        DEBUG.general.camera_position = t.position;
    }
}
