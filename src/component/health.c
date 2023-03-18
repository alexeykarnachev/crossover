#include "health.h"

Health init_health(
    float initial_value,
    int resurrection_is_active,
    float resurrection_delay
) {
    Health health = {
        .resurrection
        = {.is_active = resurrection_is_active,
           .delay = resurrection_delay,
           .countdown = 0.0},
        .initial_value = initial_value,
        .curr_value = initial_value,
        .damage_dealler = -1};
    return health;
}

Health init_default_health(void) {
    float initial_value = 1000.0;
    int resurrection_is_active = 0;
    float resurrection_delay = 5.0;
    return init_health(
        initial_value, resurrection_is_active, resurrection_delay
    );
}
