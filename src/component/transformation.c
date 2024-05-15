#include "transformation.h"

#include "../math.h"

Transformation init_transformation(Vec2 position, float orientation, float elevation) {
    Transformation transformation;
    transformation.prev_position = position;
    transformation.curr_position = position;
    transformation.prev_orientation = orientation;
    transformation.curr_orientation = orientation;
    transformation.elevation = elevation;
    return transformation;
}

void apply_transformation(Vec2 *vertices, int n_vertices, Transformation transformation) {
    for (int i = 0; i < n_vertices; ++i) {
        Vec2 *vertex = &vertices[i];
        *vertex = rotate(*vertex, vec2(0.0, 0.0), transformation.curr_orientation);
        *vertex = add(*vertex, transformation.curr_position);
    }
}

void apply_inverse_transformation(
    Vec2 *vertices, int n_vertices, Transformation transformation
) {
    for (int i = 0; i < n_vertices; ++i) {
        Vec2 *vertex = &vertices[i];
        *vertex = sub(*vertex, transformation.curr_position);
        *vertex = rotate(*vertex, vec2(0.0, 0.0), -transformation.curr_orientation);
    }
}
