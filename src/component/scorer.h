#pragma once
#include "../const.h"
#include "../math.h"
#include <stdio.h>

typedef struct ScalarScore {
    float value;
    float weight;
} ScalarScore;

typedef struct Scorer {
    struct {
        ScalarScore do_kill;
        ScalarScore do_kinematic_move;
        ScalarScore do_kinematic_rotation;
        ScalarScore do_shoot;
        ScalarScore do_hit;
        ScalarScore get_killed;
        ScalarScore get_hit;
        ScalarScore get_rb_collided;
    } scalars;

    struct {
        int grid[EXPLORATION_GRID_HEIGHT][EXPLORATION_GRID_WIDTH];
        Vec2 start_position;
        Vec2 prev_cell;
        float cell_size;
        float cell_enter_time;
        float stay_in_cell_delay;

        ScalarScore new_cell_score;
        ScalarScore old_cell_score;
        ScalarScore stay_in_cell_score;
    } kinematic_exploration;
} Scorer;

void reset_scorer(Scorer* scorer);
float get_total_score(Scorer* scorer);
void read_scorer(FILE* fp, Scorer* scorer);
void write_scorer(FILE* fp, Scorer* scorer);
void write_n_scorers(FILE* fp, Scorer* scorers, int n);
void read_n_scorers(FILE* fp, Scorer* scorers, int n);
