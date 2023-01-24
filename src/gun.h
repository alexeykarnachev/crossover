#pragma once
#include "material.h"

typedef struct Gun {
    struct bullet {
        Material material;
        float base_speed;
        float radius;
    } bullet;

    float fire_rate;
} Gun;

Gun gun(
    Material bullet_material,
    float bullet_base_speed,
    float bullet_radius,
    float fire_rate
);
