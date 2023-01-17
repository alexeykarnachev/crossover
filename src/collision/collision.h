#pragma once
#include "../math.h"
#include "../primitive.h"

typedef struct Collision {
    Vec2 mtv;
    int entity0;
    int entity1;
} Collision;

int collide_entities(int e0, int e1, Collision* out);
void resolve_collision(Collision collision);
