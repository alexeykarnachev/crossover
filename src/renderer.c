#include <glad/glad.h>

#include "./debug/debug.h"
#include "app.h"
#include "math.h"
#include "primitive.h"
#include "program.h"
#include "renderer.h"
#include "world.h"
#include <stdio.h>

static GLuint DUMMY_VAO;
static Vec3 WIREFRAME_COLOR = {0.0, 0.0, 0.0};
static Vec3 MTV_COLOR = {0.0, 1.0, 0.0};

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

static void set_uniform_rectangle(GLuint program, Rectangle rectangle) {
    set_uniform_2fv(
        program, "rectangle.position", (float*)&rectangle.position, 1
    );
    set_uniform_1f(program, "rectangle.width", rectangle.width);
    set_uniform_1f(program, "rectangle.height", rectangle.height);
}

static void set_uniform_triangle(GLuint program, Triangle triangle) {
    set_uniform_2fv(
        program, "triangle.position", (float*)&triangle.position, 1
    );
    set_uniform_2fv(program, "triangle.b", (float*)&triangle.b, 1);
    set_uniform_2fv(program, "triangle.c", (float*)&triangle.c, 1);
}

static void set_uniform_line(GLuint program, Line line) {
    set_uniform_2fv(program, "line.position", (float*)&line.position, 1);
    set_uniform_2fv(program, "line.b", (float*)&line.b, 1);
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
        set_uniform_rectangle(program, primitive.p.rectangle);
        draw_mode = GL_TRIANGLE_STRIP;
        n_points = 4;
    } else if (type & TRIANGLE_PRIMITIVE) {
        set_uniform_triangle(program, primitive.p.triangle);
        draw_mode = GL_TRIANGLE_STRIP;
        n_points = 3;
    } else if (type & LINE_PRIMITIVE) {
        set_uniform_line(program, primitive.p.line);
        draw_mode = GL_LINE_STRIP;
        n_points = 2;
    } else {
        fprintf(
            stderr,
            "ERROR: can't render the primitive with type id: "
            "%d\n",
            type
        );
    }

    set_uniform_1i(program, "type", type);
    if (DEBUG.shading.material) {
        set_uniform_3fv(
            program, "diffuse_color", (float*)&material.diffuse_color, 1
        );
        glDrawArrays(draw_mode, 0, n_points);
    }

    if (DEBUG.shading.wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        set_uniform_3fv(
            program, "diffuse_color", (float*)&WIREFRAME_COLOR, 1
        );
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
