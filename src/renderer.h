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
