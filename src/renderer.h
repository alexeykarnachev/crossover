#pragma once

typedef struct Renderer {
    GLuint guys_vao;
} Renderer;

extern Renderer RENDERER;

void create_renderer(void);
void render_gui(void);
void render_guys(void);
