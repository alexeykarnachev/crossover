#pragma once

typedef struct Health {
    float value;
    int damage_dealler;
} Health;

Health init_health(float health);
Health init_default_health(void);
