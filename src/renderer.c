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

static void set_uniform_circle(
    GLuint program, Transformation transformation, Circle circle
) {
    set_uniform_2fv(
        program, "circle.position", (float*)&transformation.position, 1
    );
    set_uniform_1f(program, "circle.radius", circle.radius);
    set_uniform_1i(program, "circle.n_polygons", N_POLYGONS_IN_CIRCLE);
}

static int set_uniform_primitive(
    GLuint program, Transformation transformation, Primitive primitive
) {
    Vec2 vertices[4];
    int n_points = get_primitive_vertices(
        primitive, transformation, vertices
    );
    set_uniform_2fv(program, "polygon.a", (float*)&vertices[0], 1);
    set_uniform_2fv(program, "polygon.b", (float*)&vertices[1], 1);
    set_uniform_2fv(program, "polygon.c", (float*)&vertices[2], 1);
    set_uniform_2fv(program, "polygon.d", (float*)&vertices[3], 1);

    return n_points;
}

static GLuint get_primitive_type_draw_mode(PrimitiveType type) {
    switch (type) {
        case CIRCLE_PRIMITIVE:
            return GL_TRIANGLE_FAN;
        case RECTANGLE_PRIMITIVE:
            return GL_TRIANGLE_STRIP;
        case TRIANGLE_PRIMITIVE:
            return GL_TRIANGLE_STRIP;
        case LINE_PRIMITIVE:
            return GL_LINE_STRIP;
        default:
            fprintf(
                stderr,
                "ERROR: can't render the primitive with type id: "
                "%d. Needs to be implemented\n",
                type
            );
            exit(1);
    }
}

static void render_primitive(
    Transformation transformation, Primitive primitive, Material material
) {
    GLuint program = PRIMITIVE_PROGRAM;
    glUseProgram(program);
    set_uniform_camera(program, WORLD.camera);

    PrimitiveType type = primitive.type;
    GLuint draw_mode = get_primitive_type_draw_mode(type);
    int n_points;
    if (type & CIRCLE_PRIMITIVE) {
        set_uniform_circle(program, transformation, primitive.p.circle);
        n_points = N_POLYGONS_IN_CIRCLE + 2;
    } else {
        n_points = set_uniform_primitive(
            program, transformation, primitive
        );
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
        if (!entity_can_be_rendered) {
            continue;
        }

        Transformation transformation = WORLD.transformation[entity];
        Primitive primitive = WORLD.primitive[entity];
        Material material = default_material();
        if (entity_has_component(entity, MATERIAL_COMPONENT)) {
            material = WORLD.material[entity];
        }
        render_primitive(transformation, primitive, material);
    }

    while (DEBUG.n_primitives > 0) {
        DEBUG.n_primitives -= 1;
        DebugPrimitive p = DEBUG.primitives[DEBUG.n_primitives];
        render_primitive(p.transformation, p.primitive, p.material);
    }
}
