#pragma once
#include "array.h"
#include "component.h"
#include "const.h"
#include "math.h"

typedef struct Collision {
    Vec2 mtv;
    int entity0;
    int entity1;
} Collision;

extern Collision COLLISIONS[MAX_N_COLLISIONS];
extern int N_COLLISIONS;

void push_collision(Collision collision);
Collision pop_collision(void);

void update_collisions(int is_playing);
void update_get_rb_collided_score(int entity);
int collide_primitives(
    Primitive primitive0,
    Transformation transformation0,
    Primitive primitive1,
    Transformation transformation1,
    Collision *collision
);
void render_colliders(void);
