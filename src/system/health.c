#include "../scene.h"
#include "../system.h"

void update_healths() {
    for (int entity = 0; entity < SCENE.n_entities; ++entity) {
        if (!check_if_entity_has_component(entity, HEALTH_COMPONENT)) {
            continue;
        }

        Health* health = &SCENE.healths[entity];

        if (health->value <= 0) {
            destroy_entity(entity);
            if (check_if_entity_has_component(entity, SCORER_COMPONENT)) {
                update_get_killed_score(entity);

                int damage_dealler = health->damage_dealler;
                if (damage_dealler != -1
                    && check_if_entity_has_component(
                        damage_dealler, SCORER_COMPONENT
                    )) {
                    update_kill_enemy_score(damage_dealler);
                }
            }
        }

        health->damage_dealler = -1;
    }
}
