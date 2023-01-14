#pragma once

typedef enum CollisionType {
    INTERSECTION_COLLISION,
    CONTAINMENT_COLLISION
} CollisionType;

typedef struct Collision {
    CollisionType type;
    int entity0;
    int entity1;
} Collision;

Collision collision(CollisionType type, int entity0, int entity1);
