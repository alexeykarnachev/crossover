#include "../scene.h"
#include "../system.h"

void update_healths() {
    for (int entity = 0; entity < SCENE.n_entities; ++entity) {
        if (!check_if_entity_has_component(entity, HEALTH_COMPONENT)) {
            continue;
        }

        if (SCENE.healths[entity] <= 0) {
            destroy_entity(entity);
        }
    }
}
