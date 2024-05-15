#include "../debug.h"
#include "../scene.h"
#include "../system.h"

void update_healths(float dt) {
    for (int entity = 0; entity < SCENE.n_entities; ++entity) {
        int has_health = check_if_entity_has_component(entity, HEALTH_COMPONENT);
        int has_hidden_health = SCENE.hiddens[entity] & HEALTH_COMPONENT;
        if (has_health == 0 && has_hidden_health == 0) {
            continue;
        }

        Health *health = &SCENE.healths[entity];
        if (health->curr_value <= 0) {
            if (DEBUG.gameplay.all_immortal == 1) {
                health->curr_value = health->initial_value;
            } else {
                int damage_dealler = health->damage_dealler;
                if (damage_dealler != -1
                    && check_if_entity_has_component(damage_dealler, SCORER_COMPONENT)) {
                    update_do_kill_score(damage_dealler);
                }

                if (health->resurrection.is_active) {
                    update_get_killed_score(entity);
                    health->curr_value = health->initial_value;
                    health->resurrection.countdown = health->resurrection.delay;
                    hide_entity(entity);
                } else if (check_if_entity_has_component(entity, SCORER_COMPONENT)) {
                    // TODO: Think about this case. What should I do if I
                    // need a scorer component (updated after the entity
                    // death). For now I just don't destroy an entity
                    // without Scorer component.
                    update_get_killed_score(entity);
                    // NOTE: Leave only the SCORER_COMPONENT,
                    // so this entity doesn't participate in the
                    // common scene update routines, but it doesn't
                    // destroyed either...
                    SCENE.components[entity] = SCORER_COMPONENT;
                } else {
                    destroy_entity(entity);
                }
            }
        }

        if (check_if_entity_hidden(entity) && health->resurrection.countdown <= 0) {
            reveal_entity(entity);
        }

        health->resurrection.countdown -= dt;
        health->damage_dealler = -1;
    }
}
