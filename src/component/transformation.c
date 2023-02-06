#include "transformation.h"

#include "../math.h"

Transformation init_transformation(Vec2 position, float orientation) {
    Transformation transformation;
    transformation.position = position;
    transformation.orientation = orientation;
    return transformation;
}

void apply_transformation(
    Vec2* vertices, int n_vertices, Transformation transformation
) {
    for (int i = 0; i < n_vertices; ++i) {
        Vec2* vertex = &vertices[i];
        *vertex = rotate(
            *vertex, vec2(0.0, 0.0), transformation.orientation
        );
        *vertex = add(*vertex, transformation.position);
    }
}

void apply_inverse_transformation(
    Vec2* vertices, int n_vertices, Transformation transformation
) {
    for (int i = 0; i < n_vertices; ++i) {
        Vec2* vertex = &vertices[i];
        *vertex = sub(*vertex, transformation.position);
        *vertex = rotate(
            *vertex, vec2(0.0, 0.0), -transformation.orientation
        );
    }
}
