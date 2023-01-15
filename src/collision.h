#pragma once
#include "primitive.h"

typedef enum CollisionType {
    TOUCH_COLLISION = 1 << 0,
    INTERSECTION_COLLISION = 1 << 1,
    CONTAINMENT_COLLISION_0 = 1 << 2,  // 0th entity is larger
    CONTAINMENT_COLLISION_1 = 1 << 3  // 1th entity is larger
} CollisionType;

typedef struct Collision {
    CollisionType type;
    int entity0;
    int entity1;
} Collision;

Collision collision(CollisionType type, int entity0, int entity1);
int collide_primitives(Primitive p0, Primitive p1, CollisionType* out);
