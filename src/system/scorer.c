#include "../scene.h"
#include "../system.h"
#include <math.h>

// TODO: All these functions could be generalized somehow...
void update_get_killed_score(int entity) {
    ScalarScore *scorer = &SCENE.scorers[entity].scalars.get_killed;
    scorer->value += scorer->weight;
}

void update_do_kill_score(int entity) {
    ScalarScore *scorer = &SCENE.scorers[entity].scalars.do_kill;
    scorer->value += scorer->weight;
}

void update_do_move_score(int entity, Vec2 step) {
    ScalarScore *scorer = &SCENE.scorers[entity].scalars.do_move;
    scorer->value += scorer->weight * length(step);
}

void update_do_rotation_score(int entity, float angular_step) {
    ScalarScore *scorer = &SCENE.scorers[entity].scalars.do_rotation;
    scorer->value += scorer->weight * fabs(angular_step);
}

void update_do_shoot_score(int entity) {
    ScalarScore *scorer = &SCENE.scorers[entity].scalars.do_shoot;
    scorer->value += scorer->weight;
}

void update_do_hit_score(int entity) {
    ScalarScore *scorer = &SCENE.scorers[entity].scalars.do_hit;
    scorer->value += scorer->weight;
}

void update_get_hit_score(int entity) {
    ScalarScore *scorer = &SCENE.scorers[entity].scalars.get_hit;
    scorer->value += scorer->weight;
}

void update_get_rb_collided_score(int entity) {
    ScalarScore *scorer = &SCENE.scorers[entity].scalars.get_rb_collided;
    scorer->value += scorer->weight;
}

void update_exploration_score(int entity, float dt) {
    Scorer *scorer = &SCENE.scorers[entity];
    Vec2 start_position = scorer->exploration.start_position;
    Vec2 curr_position = SCENE.transformations[entity].curr_position;
    Vec2 diff = sub(curr_position, start_position);
    float cell_size = scorer->exploration.cell_size;

    // TODO: Maybe reduce duplication somehow? Or maybe not...
    float diff_x = fabs(diff.x) - 0.5 * cell_size;
    int cell_x = 0;
    if (diff_x > 0) {
        cell_x = sign(diff.x) * (int)ceil(diff_x / cell_size);
    }

    float diff_y = fabs(diff.y) - 0.5 * cell_size;
    int cell_y = 0;
    if (diff_y > 0) {
        cell_y = sign(diff.y) * (int)ceil(diff_y / cell_size);
    }

    cell_x += EXPLORATION_GRID_WIDTH / 2;
    cell_y += EXPLORATION_GRID_HEIGHT / 2;
    int prev_cell_x = scorer->exploration.prev_cell.x;
    int prev_cell_y = scorer->exploration.prev_cell.y;

    if (min(cell_x, cell_y) >= 0 && cell_x < EXPLORATION_GRID_WIDTH
        && cell_y < EXPLORATION_GRID_HEIGHT) {
        int *is_visited = &scorer->exploration.grid[cell_x][cell_y];
        if (*is_visited == 0) {
            *is_visited = 1;
            float weight = scorer->exploration.new_cell_score.weight;
            scorer->exploration.new_cell_score.value += weight;
            scorer->exploration.cell_enter_time = SCENE.time;
        } else if (cell_x != prev_cell_x || cell_y != prev_cell_y) {
            float weight = scorer->exploration.old_cell_score.weight;
            scorer->exploration.old_cell_score.value += weight;
            scorer->exploration.cell_enter_time = SCENE.time;
        } else {
            float weight = scorer->exploration.stay_in_cell_score.weight;
            float stay_in_cell_time = SCENE.time - scorer->exploration.cell_enter_time;
            if (stay_in_cell_time > scorer->exploration.stay_in_cell_delay) {
                scorer->exploration.stay_in_cell_score.value += weight * dt;
            }
        }
    }

    scorer->exploration.prev_cell = vec2(cell_x, cell_y);
}
