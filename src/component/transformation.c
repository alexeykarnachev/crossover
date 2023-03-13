#include "transformation.h"

#include "../math.h"

Transformation init_transformation(Vec2 position, float orientation) {
    Transformation transformation;
    transformation.prev_position = position;
    transformation.curr_position = position;
    transformation.prev_orientation = orientation;
    transformation.curr_orientation = orientation;
    return transformation;
}

void update_position(Transformation* transformation, Vec2 position) {
    transformation->prev_position = transformation->curr_position;
    transformation->curr_position = position;
}

void update_orientation(
    Transformation* transformation, float orientation
) {
    transformation->prev_orientation = transformation->curr_orientation;
    transformation->curr_orientation = orientation;
}

int check_if_transformation_changed(Transformation transformation) {
    int position_changed = neq(
        transformation.prev_position, transformation.curr_position
    );
    int orientation_changed = transformation.prev_orientation
                              != transformation.curr_orientation;
    return position_changed || orientation_changed;
}

void apply_transformation(
    Vec2* vertices, int n_vertices, Transformation transformation
) {
    for (int i = 0; i < n_vertices; ++i) {
        Vec2* vertex = &vertices[i];
        *vertex = rotate(
            *vertex, vec2(0.0, 0.0), transformation.curr_orientation
        );
        *vertex = add(*vertex, transformation.curr_position);
    }
}

void apply_inverse_transformation(
    Vec2* vertices, int n_vertices, Transformation transformation
) {
    for (int i = 0; i < n_vertices; ++i) {
        Vec2* vertex = &vertices[i];
        *vertex = sub(*vertex, transformation.curr_position);
        *vertex = rotate(
            *vertex, vec2(0.0, 0.0), -transformation.curr_orientation
        );
    }
}
