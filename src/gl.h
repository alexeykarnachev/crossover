#pragma once

typedef enum FILL_TYPE { LINE, FILL } FILL_TYPE;

void init_renderer(void);
void render_scene(float dt);
