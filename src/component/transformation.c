#include "../component.h"
#include "../math.h"

Transformation init_transformation(Vec2 position, float orientation) {
    Transformation t = {position, orientation};
    return t;
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
