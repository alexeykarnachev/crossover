#include <glad/glad.h>

#include "./debug/debug.h"
#include "app.h"
#include "component.h"
#include "math.h"
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

static void set_uniform_camera(GLuint program, Transformation camera) {
    CameraFrustum frustum = get_camera_frustum();
    Vec2 view_size = sub(frustum.top_right, frustum.bot_left);
    set_uniform_2fv(
        program, "camera.position", (float*)&camera.position, 1
    );
    set_uniform_1f(program, "camera.view_width", view_size.x);
    set_uniform_1f(program, "camera.view_height", view_size.y);
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
    int nv = get_primitive_vertices(primitive, vertices);
    apply_transformation(vertices, nv, transformation);
    set_uniform_2fv(program, "polygon.a", (float*)&vertices[0], 1);
    set_uniform_2fv(program, "polygon.b", (float*)&vertices[1], 1);
    set_uniform_2fv(program, "polygon.c", (float*)&vertices[2], 1);
    set_uniform_2fv(program, "polygon.d", (float*)&vertices[3], 1);

    return nv;
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

typedef struct RenderCall {
    int n_points;
    GLuint draw_mode;
} RenderCall;

static RenderCall prepare_primitive_render_call(
    Transformation transformation, Primitive primitive, Material material
) {
    GLuint program = PRIMITIVE_PROGRAM;
    glUseProgram(program);
    PrimitiveType type = primitive.type;

    set_uniform_camera(program, WORLD.transformation[WORLD.camera]);
    set_uniform_1i(program, "type", type);
    set_uniform_3fv(
        program, "diffuse_color", (float*)&material.diffuse_color, 1
    );

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

    RenderCall render_call;
    render_call.n_points = n_points;
    render_call.draw_mode = draw_mode;

    return render_call;
}

static void execute_render_call(RenderCall render_call, int fill_type) {
    GLuint fill_type_gl;
    switch (fill_type) {
        case LINE: {
            fill_type_gl = GL_LINE;
            break;
        }
        case FILL: {
            fill_type_gl = GL_FILL;
            break;
        }
        default:
            fill_type_gl = GL_FILL;
    }

    glPolygonMode(GL_FRONT_AND_BACK, fill_type_gl);
    glDrawArrays(render_call.draw_mode, 0, render_call.n_points);
}

void render_world(void) {
    // -------------------------------------------------------------------
    // Render primitives
    glDisable(GL_CULL_FACE);
    glViewport(0, 0, APP.window_width, APP.window_height);
    glBindVertexArray(DUMMY_VAO);

    for (int entity = 0; entity < WORLD.n_entities; ++entity) {
        if (!entity_can_be_rendered(entity)) {
            continue;
        }

        Transformation transformation = WORLD.transformation[entity];
        Primitive primitive = WORLD.primitive[entity];
        Material material = init_material(GRAY_COLOR);
        if (entity_has_component(entity, MATERIAL_COMPONENT)) {
            material = WORLD.material[entity];
        }

        RenderCall render_call = prepare_primitive_render_call(
            transformation, primitive, material
        );

        if (DEBUG.shading.materials) {
            execute_render_call(render_call, FILL);
        }

        if (DEBUG.shading.wireframe) {
            execute_render_call(render_call, LINE);
        }
    }

    while (DEBUG.n_primitives > 0) {
        DEBUG.n_primitives -= 1;
        DebugPrimitive p = DEBUG.primitives[DEBUG.n_primitives];
        RenderCall render_call = prepare_primitive_render_call(
            p.transformation, p.primitive, init_material(p.color)
        );
        execute_render_call(render_call, p.fill_type);
    }
}
