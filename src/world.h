#pragma once
#include "const.h"
#include "guy.h"
#include <stddef.h>

typedef struct World {
    size_t n_guys;
    Guy guys[MAX_N_GUYS];
} World;

extern World WORLD;

void create_world(void);
void spawn_guy(Guy guy);
