#pragma once
#include "../math.h"

typedef struct Light {
    Vec3 color;
    Vec2 direction;
    int is_dir;
} Light;

Light init_light(Vec3 color, Vec2 direction, int is_dir);
