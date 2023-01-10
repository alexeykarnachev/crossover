#pragma once
#include "guy.h"
#include <stddef.h>

#define MAX_N_GUYS 1024

typedef struct World {
    size_t n_guys;
    Guy guys[MAX_N_GUYS];
} World;

extern World WORLD;
