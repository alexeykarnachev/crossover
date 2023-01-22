#pragma once
#include "math.h"
#include "primitive.h"

typedef struct Collision {
    Vec2 mtv;
    int entity0;
    int entity1;
} Collision;

int collide_primitives(Primitive p0, Primitive p1, Collision* collision);
void collide_with_world(int entity);
void resolve_collision(Collision collision);
Line project_circle_on_axis(Circle circle, Vec2 axis);
Line project_polygon_on_axis(Vec2 vertices[], int n, Vec2 axis);
