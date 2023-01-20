#include "movement.h"

#include "math.h"
#include "world.h"

Movement movement(float speed, float rotation_speed) {
    Movement m = {vec2(0.0, 0.0), speed, rotation_speed};
    return m;
}

void move_entity(int entity, float dt) {
    if (!entity_has_component(entity, MOVEMENT_COMPONENT)) {
        return;
    }

    Movement m = WORLD.movement[entity];
    if (length(m.direction) > EPS) {
        Vec2 t = scale(normalize(m.direction), m.speed * dt);
        transform_entity(entity, t, 0.0);
    }
}
