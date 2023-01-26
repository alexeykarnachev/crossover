#pragma once
#include "math.h"
#include "primitive.h"
#include "transformation.h"

typedef struct Collision {
    Vec2 mtv;
    int entity0;
    int entity1;
} Collision;

int collide_primitives(
    Primitive p0,
    Transformation t0,
    Primitive p1,
    Transformation t1,
    Collision* collision
);

void compute_collisions();
void resolve_collisions();
