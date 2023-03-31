#include "light.h"

#include "math.h"

Light init_light(
    Vec3 color, Vec3 attenuation, Vec2 direction, float power, int is_dir
) {
    Light light = {
        .color = color,
        .attenuation = attenuation,
        .direction = direction,
        .power = power,
        .is_dir = is_dir};
    return light;
}
