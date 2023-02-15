#include "../scene.h"
#include "../system.h"

void update_get_killed_score(int entity) {
    Scorer* scorer = &SCENE.scorers[entity];
    scorer->score += scorer->weight.get_killed;
}

void update_kill_enemy_score(int entity) {
    Scorer* scorer = &SCENE.scorers[entity];
    scorer->score += scorer->weight.kill_enemy;
}

void update_do_kinematic_move_score(int entity, Vec2 step) {
    Scorer* scorer = &SCENE.scorers[entity];
    scorer->score += scorer->weight.do_kinematic_move * length(step);
}

void update_do_shoot_score(int entity) {
    Scorer* scorer = &SCENE.scorers[entity];
    scorer->score += scorer->weight.do_shoot;
}

void update_loose_health_score(int entity, float damage) {
    Scorer* scorer = &SCENE.scorers[entity];
    scorer->score += scorer->weight.loose_health * damage;
}

void update_hit_enemy_score(int entity, float damage) {
    Scorer* scorer = &SCENE.scorers[entity];
    scorer->score += scorer->weight.hit_enemy * damage;
}
