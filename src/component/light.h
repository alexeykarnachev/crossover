#pragma once
#include "../math.h"

typedef struct Light {
    Vec3 color;
    Vec3 attenuation;
    Vec2 direction;
    float radius;
    float power;
    int is_dir;
} Light;

Light init_light(
    Vec3 color,
    Vec3 attenuation,
    Vec2 direction,
    float radius,
    float power,
    int is_dir
);
