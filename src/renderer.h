#pragma once

#define PRIMITIVE_VERT_SHADER "./assets/shaders/primitive.vert"
#define PRIMITIVE_FRAG_SHADER "./assets/shaders/primitive.frag"
#define SCREEN_RECT_VERT_SHADER "./assets/shaders/screen_rect.vert"
#define COLOR_FRAG_SHADER "./assets/shaders/color.frag"
#define LIGHT_MASK_FRAG_SHADER "./assets/shaders/light_mask.frag"

typedef enum FILL_TYPE { LINE, FILL } FILL_TYPE;

void init_gl(void);
void render_scene(float dt);
