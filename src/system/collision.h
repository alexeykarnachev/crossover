#pragma once
#include "../component/component.h"
#include "../math.h"

typedef struct Collision {
    Vec2 mtv;
    int entity0;
    int entity1;
} Collision;

void update_collision();
