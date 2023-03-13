#include "health.h"

Health init_health(float initial_value) {
    Health health = {
        .initial_value = initial_value,
        .curr_value = initial_value,
        .damage_dealler = -1};
    return health;
}

Health init_default_health(void) {
    return init_health(1000.0);
}
