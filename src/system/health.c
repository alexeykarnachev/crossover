#include "../system.h"
#include "../world.h"

void update_healths() {
    for (int entity = 0; entity < WORLD.n_entities; ++entity) {
        if (!entity_has_health(entity)) {
            continue;
        }

        if (WORLD.healths[entity] <= 0) {
            destroy_entity(entity);
        }
    }
}
