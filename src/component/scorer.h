#pragma once

typedef struct ScalarScore {
    float value;
    float weight;
} ScalarScore;

typedef struct Scorer {
    struct {
        ScalarScore do_kill;
        ScalarScore do_kinematic_move;
        ScalarScore do_shoot;
        ScalarScore do_hit;
        ScalarScore get_killed;
        ScalarScore get_hit;
        ScalarScore get_rb_collided;
    } scalars;
} Scorer;

void reset_scorer(Scorer* scorer);
float get_total_score(Scorer* scorer);
