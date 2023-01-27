#pragma once

typedef struct Gun {
    struct bullet {
        float ttl;
        float speed;
    } bullet;

    float fire_rate;
    float last_time_shoot;
} Gun;

Gun gun(float bullet_ttl, float bullet_speed, float fire_rate);
