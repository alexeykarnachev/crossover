#pragma once

typedef struct Health {
    struct resurrection {
        int is_active;
        float delay;
        float countdown;
    } resurrection;

    float initial_value;
    float curr_value;

    int damage_dealler;
} Health;

Health init_health(
    float initial_value,
    int resurrection_is_active,
    float resurrection_delay
);
Health init_default_health(void);
