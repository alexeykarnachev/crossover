#include <glad/glad.h>

#include "./debug/debug.h"
#include "app.h"
#include "math.h"
#include "primitive.h"
#include "program.h"
#include "renderer.h"
#include "world.h"
#include <stdio.h>
#include <stdlib.h>

static GLuint DUMMY_VAO;

void init_renderer(void) {
    glCreateVertexArrays(1, &DUMMY_VAO);
    init_all_programs();
}

static const int N_POLYGONS_IN_CIRCLE = 16;

static void set_uniform_camera(GLuint program, Camera camera) {
    float aspect_ratio = (float)APP.window_width / APP.window_height;
    set_uniform_2fv(
        program, "camera.position", (float*)&camera.position, 1
    );
    set_uniform_1f(program, "camera.aspect_ratio", aspect_ratio);
    set_uniform_1f(program, "camera.elevation", camera.elevation);
}

static void set_uniform_circle(GLuint program, Circle circle) {
    set_uniform_2fv(
        program, "circle.position", (float*)&circle.position, 1
    );
    set_uniform_1f(program, "circle.radius", circle.radius);
    set_uniform_1i(program, "circle.n_polygons", N_POLYGONS_IN_CIRCLE);
}

static int set_uniform_primitive(GLuint program, Primitive primitive) {
    Vec2 vertices[4];
    int n_points = get_primitive_vertices(primitive, vertices);
    set_uniform_2fv(program, "polygon.a", (float*)&vertices[0], 1);
    set_uniform_2fv(program, "polygon.b", (float*)&vertices[1], 1);
    set_uniform_2fv(program, "polygon.c", (float*)&vertices[2], 1);
    set_uniform_2fv(program, "polygon.d", (float*)&vertices[3], 1);

    return n_points;
}

static void render_primitive(Primitive primitive, Material material) {
    GLuint program = PRIMITIVE_PROGRAM;
    glUseProgram(program);
    set_uniform_camera(program, WORLD.camera);

    PrimitiveType type = primitive.type;
    GLuint draw_mode;
    int n_points;
    if (type & CIRCLE_PRIMITIVE) {
        set_uniform_circle(program, primitive.p.circle);
        draw_mode = GL_TRIANGLE_FAN;
        n_points = N_POLYGONS_IN_CIRCLE + 2;
    } else if (type & RECTANGLE_PRIMITIVE) {
        n_points = set_uniform_primitive(program, primitive);
        draw_mode = GL_TRIANGLE_STRIP;
    } else if (type & TRIANGLE_PRIMITIVE) {
        n_points = set_uniform_primitive(program, primitive);
        draw_mode = GL_TRIANGLE_STRIP;
    } else if (type & LINE_PRIMITIVE) {
        n_points = set_uniform_primitive(program, primitive);
        draw_mode = GL_LINE_STRIP;
    } else {
        fprintf(
            stderr,
            "ERROR: can't render the primitive with type id: "
            "%d. Needs to be implemented\n",
            type
        );
        exit(1);
    }

    set_uniform_1i(program, "type", type);
    if (DEBUG.shading.materials) {
        set_uniform_3fv(
            program, "diffuse_color", (float*)&material.diffuse_color, 1
        );
        glDrawArrays(draw_mode, 0, n_points);
    }

    if (DEBUG.shading.wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        set_uniform_3fv(program, "diffuse_color", (float*)&GRAY_COLOR, 1);
        glDrawArrays(draw_mode, 0, n_points);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void render_world(void) {
    // -------------------------------------------------------------------
    // Render primitives
    glDisable(GL_CULL_FACE);
    glViewport(0, 0, APP.window_width, APP.window_height);
    glBindVertexArray(DUMMY_VAO);

    for (int entity = 0; entity < WORLD.n_entities; ++entity) {
        if (!entity_has_component(entity, PRIMITIVE_COMPONENT)) {
            continue;
        }

        Primitive primitive = WORLD.primitive[entity];
        Material material = default_material();
        if (entity_has_component(entity, MATERIAL_COMPONENT)) {
            material = WORLD.material[entity];
        }
        render_primitive(primitive, material);
    }

    while (DEBUG.n_render_commands > 0) {
        DEBUG.n_render_commands -= 1;
        RenderCommand c = DEBUG.render_commands[DEBUG.n_render_commands];
        if (c.type == PRIMITIVE_RENDER) {
            Primitive primitive = c.command.render_primitive.primitive;
            Material material = c.command.render_primitive.material;
            render_primitive(primitive, material);
        }
    }
}

RenderCommand render_primitive_command(
    Primitive primitive, Material material
) {
    RenderCommand render_command = {
        PRIMITIVE_RENDER, {primitive, material}};
    return render_command;
}

RenderCommand render_circle_command(Circle circle, Material material) {
    return render_primitive_command(circle_primitive(circle), material);
}

RenderCommand render_rectangle_command(
    Rectangle rectangle, Material material
) {
    return render_primitive_command(
        rectangle_primitive(rectangle), material
    );
}

RenderCommand render_triangle_command(
    Triangle triangle, Material material
) {
    return render_primitive_command(
        triangle_primitive(triangle), material
    );
}

RenderCommand render_line_command(Line line, Material material) {
    return render_primitive_command(line_primitive(line), material);
}
