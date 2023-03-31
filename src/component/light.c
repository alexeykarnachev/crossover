#include "light.h"

#include "math.h"

Light init_light(Vec3 color, Vec2 direction, int is_dir) {
    Light light = {
        .color = color, .direction = direction, .is_dir = is_dir};
    return light;
}
