#include "transformation.h"

#include "math.h"

Transformation transformation(Vec2 position, float orientation) {
    Transformation t = {position, orientation};
    return t;
}

Vec2 transform(Vec2 v, Transformation t) {
    v = rotate(v, vec2(0.0, 0.0), t.orientation);
    v = add(v, t.position);
    return v;
}
