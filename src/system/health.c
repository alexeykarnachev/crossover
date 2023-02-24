#include "../scene.h"
#include "../system.h"

void update_healths() {
    for (int entity = 0; entity < SCENE.n_entities; ++entity) {
        if (!check_if_entity_has_component(entity, HEALTH_COMPONENT)) {
            continue;
        }

        Health* health = &SCENE.healths[entity];

        if (health->value <= 0) {

            int damage_dealler = health->damage_dealler;
            if (damage_dealler != -1
                && check_if_entity_has_component(
                    damage_dealler, SCORER_COMPONENT
                )) {
                update_kill_score(damage_dealler);
            }

            // TODO: Think about this case. What should I do if I need a
            // scorer component (updated after the entity death).
            // For now I just don't destroy an entity without Scorer
            // component.
            if (check_if_entity_has_component(entity, SCORER_COMPONENT)) {
                update_get_killed_score(entity);
                // Leave only the SCORER_COMPONENT, so this entity doesn't
                // participate in the common scene update routines, but it
                // doesn't destroyed either...
                SCENE.components[entity] = SCORER_COMPONENT;
            } else {
                destroy_entity(entity);
            }
        }

        health->damage_dealler = -1;
    }
}
