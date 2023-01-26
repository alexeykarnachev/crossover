#pragma once
#include "ttl.h"

typedef struct Gun {
    struct bullet {
        TTL ttl;
        float base_speed;
    } bullet;

    float fire_rate;
    float last_time_shoot;
} Gun;

Gun gun(TTL bullet_ttl, float bullet_base_speed, float fire_rate);
