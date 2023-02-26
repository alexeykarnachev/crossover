#include "scorer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: Could be generalized with macros or loop or something like this
void reset_scorer(Scorer* scorer) {
    scorer->scalars.do_kill.value = 0.0;
    scorer->scalars.do_kinematic_move.value = 0.0;
    scorer->scalars.do_kinematic_rotation.value = 0.0;
    scorer->scalars.do_shoot.value = 0.0;
    scorer->scalars.do_hit.value = 0.0;
    scorer->scalars.get_killed.value = 0.0;
    scorer->scalars.get_hit.value = 0.0;
    scorer->scalars.get_rb_collided.value = 0.0;

    memset(scorer->exploration.grid, 0, sizeof(scorer->exploration.grid));
    scorer->exploration.score.value = 0.0;
}

// TODO: Could be generalized with macros or loop or something like this
float get_total_score(Scorer* scorer) {
    float total_score = 0.0;
    total_score += scorer->scalars.do_kill.value;
    total_score += scorer->scalars.do_kinematic_move.value;
    total_score += scorer->scalars.do_kinematic_rotation.value;
    total_score += scorer->scalars.do_shoot.value;
    total_score += scorer->scalars.do_hit.value;
    total_score += scorer->scalars.get_killed.value;
    total_score += scorer->scalars.get_hit.value;
    total_score += scorer->scalars.get_rb_collided.value;
    total_score += scorer->exploration.score.value;

    return total_score;
}

void read_scorer(FILE* fp, Scorer* scorer) {
    memset(scorer, 0, sizeof(Scorer));

    fread(&scorer->scalars, sizeof(scorer->scalars), 1, fp);
    int n_explored_cells;
    fread(&n_explored_cells, sizeof(int), 1, fp);

    int* explored_cells = (int*)malloc(n_explored_cells * sizeof(int));
    fread(explored_cells, sizeof(int), n_explored_cells, fp);
    for (int i = 0; i < n_explored_cells; ++i) {
        int cell = explored_cells[i];
        int row = cell / EXPLORATION_GRID_WIDTH;
        int col = cell - row * EXPLORATION_GRID_WIDTH;
        scorer->exploration.grid[row][col] = 1;
    }

    fread(&scorer->exploration.start_position, sizeof(int), 2, fp);
    fread(&scorer->exploration.cell_size, sizeof(float), 1, fp);
    fread(&scorer->exploration.score, sizeof(ScalarScore), 1, fp);

    free(explored_cells);
}

void write_scorer(FILE* fp, Scorer* scorer) {
    int* explored_cells = (int*)malloc(sizeof(scorer->exploration.grid));
    int n_explored_cells = 0;
    for (int i = 0; i < EXPLORATION_GRID_HEIGHT; ++i) {
        for (int j = 0; j < EXPLORATION_GRID_WIDTH; ++j) {
            int is_explored = scorer->exploration.grid[i][j];
            if (is_explored) {
                int idx = i * EXPLORATION_GRID_WIDTH + j;
                explored_cells[n_explored_cells++] = idx;
            }
        }
    }

    fwrite(&scorer->scalars, sizeof(scorer->scalars), 1, fp);
    fwrite(&n_explored_cells, sizeof(int), 1, fp);
    fwrite(explored_cells, sizeof(int), n_explored_cells, fp);
    fwrite(&scorer->exploration.start_position, sizeof(int), 2, fp);
    fwrite(&scorer->exploration.cell_size, sizeof(float), 1, fp);
    fwrite(&scorer->exploration.score, sizeof(ScalarScore), 1, fp);

    free(explored_cells);
}

void write_n_scorers(FILE* fp, Scorer* scorers, int n) {
    for (int i = 0; i < n; ++i) {
        write_scorer(fp, scorers++);
    }
}

void read_n_scorers(FILE* fp, Scorer* scorers, int n) {
    for (int i = 0; i < n; ++i) {
        read_scorer(fp, scorers++);
    }
}
