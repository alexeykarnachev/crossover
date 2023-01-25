#pragma once
#include "material.h"
#include "ttl.h"

typedef struct Gun {
    struct bullet {
        Material material;
        TTL ttl;
        float base_speed;
        float radius;
    } bullet;

    float fire_rate;
    float last_time_shoot;
} Gun;

Gun gun(
    Material bullet_material,
    TTL bullet_ttl,
    float bullet_base_speed,
    float bullet_radius,
    float fire_rate
);
