#pragma once

typedef struct Scorer {
    struct {
        float get_killed;
        float kill_enemy;
        float do_kinematic_move;
        float do_shoot;
        float receive_damage;
        float deal_damage;
        float hit_enemy;
        float get_hit;
        float get_rb_collided;
    } weight;
    float value;
} Scorer;

void reset_scorer(Scorer* scorer);
