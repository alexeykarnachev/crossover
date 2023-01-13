#include "world.h"

#include "app.h"
#include "const.h"
#include "math.h"
#include "stdio.h"
#include "transformation.h"

World WORLD;

void init_world(void) {
    WORLD.n_entities = 0;
    WORLD.player = -1;
}

void update_world(void) {
    if (WORLD.player != -1) {
        if (APP.key_states[GLFW_KEY_W]) {
            printf("W\n");
        }
    }
}

int spawn_guy(Transformation transformation) {
    int entity = -1;
    if (WORLD.n_entities < MAX_N_ENTITIES) {
        entity = WORLD.n_entities++;
    } else {
        fprintf(stderr, "ERROR: Can't spawn more guys!");
    }

    WORLD.transformation[entity] = transformation;
    WORLD.velocity[entity] = vec2(0.0, 0.0);
    WORLD.collider[entity] = 1;
    WORLD.circle[entity] = 1;

    return entity;
}
