#include "health.h"

Health init_health(float value) {
    Health health = {.value = value, .damage_dealler = -1};
    return health;
}

Health init_default_health(void) {
    return init_health(1000.0);
}
