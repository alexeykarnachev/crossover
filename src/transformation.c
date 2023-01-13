#include "transformation.h"

Transformation transformation(
    Vec2 position, Vec2 scale, float orientation
) {
    Transformation t = {position, scale, orientation};
    return t;
}
