#pragma once

#define PRIMITIVE_VERT_SHADER "./assets/shaders/primitive.vert"
#define PRIMITIVE_FRAG_SHADER "./assets/shaders/primitive.frag"

typedef enum FILL_TYPE { LINE, FILL } FILL_TYPE;

void init_gl(void);
void render_scene(float dt);
