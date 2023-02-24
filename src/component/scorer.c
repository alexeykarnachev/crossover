#include "scorer.h"

#include <string.h>

// TODO: Could be generalized with macros or loop or something like this
void reset_scorer(Scorer* scorer) {
    scorer->scalars.do_kill.value = 0.0;
    scorer->scalars.do_kinematic_move.value = 0.0;
    scorer->scalars.do_shoot.value = 0.0;
    scorer->scalars.do_hit.value = 0.0;
    scorer->scalars.get_killed.value = 0.0;
    scorer->scalars.get_hit.value = 0.0;
    scorer->scalars.get_rb_collided.value = 0.0;
}

// TODO: Could be generalized with macros or loop or something like this
float get_total_score(Scorer* scorer) {
    float total_score = 0.0;
    total_score += scorer->scalars.do_kill.value;
    total_score += scorer->scalars.do_kinematic_move.value;
    total_score += scorer->scalars.do_shoot.value;
    total_score += scorer->scalars.do_hit.value;
    total_score += scorer->scalars.get_killed.value;
    total_score += scorer->scalars.get_hit.value;
    total_score += scorer->scalars.get_rb_collided.value;

    return total_score;
}
