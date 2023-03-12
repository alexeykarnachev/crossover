#pragma once

typedef struct Health {
    float initial_value;
    float current_value;
    int damage_dealler;
} Health;

Health init_health(float initial_value);
Health init_default_health(void);
