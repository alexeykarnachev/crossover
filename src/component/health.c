#include "health.h"

Health init_health(float health) {
    Health h = {.health = health, .damage_dealler = -1};
    return h;
}

Health init_default_health(void) {
    return init_health(1000.0);
}
