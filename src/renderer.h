#pragma once
#include "material.h"
#include "primitive.h"

typedef enum RenderCommandType {
    PRIMITIVE_RENDER = 1 << 0
} RenderCommandType;

typedef struct RenderPrimitiveCommand {
    Primitive primitive;
    Material material;
} RenderPrimitiveCommand;

typedef struct RenderCommand {
    RenderCommandType type;
    union {
        RenderPrimitiveCommand render_primitive;
    } command;
} RenderCommand;

void init_renderer(void);
void render_world(void);
RenderCommand render_primitive_command(
    Primitive primitive, Material material
);
RenderCommand render_circle_command(Circle circle, Material material);
RenderCommand render_rectangle_command(
    Rectangle rectangle, Material material
);
RenderCommand render_triangle_command(
    Triangle triangle, Material material
);
RenderCommand render_line_command(Line line, Material material);
