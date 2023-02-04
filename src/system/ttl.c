#include "../debug.h"
#include "../scene.h"
#include "../system.h"

void update_ttls(float dt) {
    for (int entity = 0; entity < SCENE.n_entities; ++entity) {
        if (!check_if_entity_has_component(entity, TTL_COMPONENT)) {
            continue;
        }

        SCENE.ttls[entity] -= dt;
        if (SCENE.ttls[entity] < 0) {
            destroy_entity(entity);
        }
    }
}
