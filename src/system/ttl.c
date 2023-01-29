#include "../system.h"
#include "../world.h"

void update_ttls(float dt) {
    for (int entity = 0; entity < WORLD.n_entities; ++entity) {
        if (!entity_has_component(entity, TTL_COMPONENT)) {
            continue;
        }

        WORLD.ttls[entity] -= dt;
        if (WORLD.ttls[entity] < 0) {
            destroy_entity(entity);
        }
    }
}
