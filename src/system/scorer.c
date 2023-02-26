#include "../scene.h"
#include "../system.h"
#include <math.h>

// TODO: All these functions could be generalized somehow...
void update_get_killed_score(int entity) {
    ScalarScore* scorer = &SCENE.scorers[entity].scalars.get_killed;
    scorer->value += scorer->weight;
}

void update_do_kill_score(int entity) {
    ScalarScore* scorer = &SCENE.scorers[entity].scalars.do_kill;
    scorer->value += scorer->weight;
}

void update_do_kinematic_move_score(int entity, Vec2 step) {
    ScalarScore* scorer = &SCENE.scorers[entity].scalars.do_kinematic_move;
    scorer->value += scorer->weight * length(step);
}

void update_do_kinematic_rotation_score(int entity, float angular_step) {
    ScalarScore* scorer
        = &SCENE.scorers[entity].scalars.do_kinematic_rotation;
    scorer->value += scorer->weight * fabs(angular_step);
}

void update_do_shoot_score(int entity) {
    ScalarScore* scorer = &SCENE.scorers[entity].scalars.do_shoot;
    scorer->value += scorer->weight;
}

void update_do_hit_score(int entity) {
    ScalarScore* scorer = &SCENE.scorers[entity].scalars.do_hit;
    scorer->value += scorer->weight;
}

void update_get_hit_score(int entity) {
    ScalarScore* scorer = &SCENE.scorers[entity].scalars.get_hit;
    scorer->value += scorer->weight;
}

void update_get_rb_collided_score(int entity) {
    ScalarScore* scorer = &SCENE.scorers[entity].scalars.get_rb_collided;
    scorer->value += scorer->weight;
}

void update_exploration_score(int entity) {
    Scorer* scorer = &SCENE.scorers[entity];
    // if (scorer->exploration.is_initialized == 0) {
    //     memset(
    //         scorer->exploration.grid,
    //         0,
    //         sizeof(scorer->exploration.grid)
    //     );
    //     scorer->exploration.score.value = 0.0;
    //     scorer->exploration.start_position
    //         = SCENE.transformations[entity].position;
    //     scorer->exploration.is_initialized = 1;
    // }
}
