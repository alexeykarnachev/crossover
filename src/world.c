#include "world.h"

#include "const.h"
#include "guy.h"
#include "stdio.h"

World WORLD;

void create_world(void) {
    WORLD.n_guys = 0;
}

void spawn_guy(Guy guy) {
    if (WORLD.n_guys < MAX_N_GUYS) {
        WORLD.guys[WORLD.n_guys++] = guy;
    } else {
        fprintf(stderr, "[WARING] Can't spawn more guys!");
    }
}
