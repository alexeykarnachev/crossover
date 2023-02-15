#pragma once

typedef struct Scorer {
    struct {
        float do_kinematic_move;
        float do_shoot;
        float loose_health;
        float get_killed;
        float hit_enemy;
        float kill_enemy;
    } weight;
    int score;
} Scorer;

void reset_scorer(Scorer* scorer);
