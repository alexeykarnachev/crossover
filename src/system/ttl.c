#include "../system.h"
#include "../world.h"

void update_ttl(float dt) {
    for (int entity = 0; entity < WORLD.n_entities; ++entity) {
        if (!entity_has_ttl(entity)) {
            continue;
        }

        WORLD.ttl[entity] -= dt;
        if (WORLD.ttl[entity] < 0) {
            destroy_entity(entity);
        }
    }
}
