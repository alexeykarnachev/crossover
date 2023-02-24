#include "../scene.h"
#include "../system.h"

void update_get_killed_score(int entity) {
    Scorer* scorer = &SCENE.scorers[entity];
    scorer->value += scorer->weight.get_killed;
}

void update_kill_enemy_score(int entity) {
    Scorer* scorer = &SCENE.scorers[entity];
    scorer->value += scorer->weight.kill_enemy;
}

void update_do_kinematic_move_score(int entity, Vec2 step) {
    Scorer* scorer = &SCENE.scorers[entity];
    scorer->value += scorer->weight.do_kinematic_move * length(step);
}

void update_do_shoot_score(int entity) {
    Scorer* scorer = &SCENE.scorers[entity];
    scorer->value += scorer->weight.do_shoot;
}

void update_receive_damage_score(int entity, float damage) {
    Scorer* scorer = &SCENE.scorers[entity];
    scorer->value += scorer->weight.receive_damage * damage;
}

void update_deal_damage_score(int entity, float damage) {
    Scorer* scorer = &SCENE.scorers[entity];
    scorer->value += scorer->weight.deal_damage * damage;
}

void update_hit_enemy_score(int entity) {
    Scorer* scorer = &SCENE.scorers[entity];
    scorer->value += scorer->weight.hit_enemy;
}

void update_get_hit_score(int entity) {
    Scorer* scorer = &SCENE.scorers[entity];
    scorer->value += scorer->weight.get_hit;
}

void update_get_rb_collided_score(int entity) {
    Scorer* scorer = &SCENE.scorers[entity];
    scorer->value += scorer->weight.get_rb_collided;
}
