#pragma once
#include "material.h"
#include "primitive.h"

typedef enum FILL_TYPE { LINE, FILL } FILL_TYPE;

void init_renderer(void);
void render_world(void);
