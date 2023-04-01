#pragma once

#define CIRCLE_VERT_SHADER "./assets/shaders/circle.vert"
#define MATERIAL_FRAG_SHADER "./assets/shaders/material.frag"

// #define COLOR_FRAG_SHADER "./assets/shaders/color.frag"
// #define LIGHT_MASK_FRAG_SHADER "./assets/shaders/light_mask.frag"

typedef enum FILL_TYPE { LINE, FILL } FILL_TYPE;

void init_gl(void);
void render_scene(float dt);
