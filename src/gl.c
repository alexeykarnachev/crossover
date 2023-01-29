#include "gl.h"
#include <glad/glad.h>

#include "app.h"
#include "component.h"
#include "debug.h"
#include "math.h"
#include "system.h"
#include "utils.h"
#include "world.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* PRIMITIVE_VERT_SHADER
    = "./assets/shaders/primitive.vert";
static const char* PRIMITIVE_FRAG_SHADER
    = "./assets/shaders/primitive.frag";

GLuint PRIMITIVE_PROGRAM;

static int compile_program_source(
    const GLchar* source, GLenum shader_type, GLuint* shader
) {
    *shader = glCreateShader(shader_type);
    glShaderSource(*shader, 1, &source, NULL);
    glCompileShader(*shader);

    GLint is_compiled;
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &is_compiled);
    if (is_compiled != GL_TRUE) {
        GLchar message[1024];
        GLsizei message_size = 0;
        glGetShaderInfoLog(
            *shader, sizeof(message), &message_size, message
        );
        fprintf(
            stderr,
            "ERROR: failed to compile the shader source %s\n",
            message
        );
        return 0;
    }
    return 1;
}

static int compile_program_file(
    const char* file_path, GLenum shader_type, GLuint* shader
) {
    const char* source = read_cstr_file(file_path, "r", NULL);
    if (source == NULL) {
        fprintf(
            stderr,
            "ERROR: failed to read the shader source file `%s`: %s\n",
            file_path,
            strerror(errno)
        );
        errno = 0;
        return 0;
    }

    int is_compiled = compile_program_source(source, shader_type, shader);
    free((char*)source);

    if (!is_compiled) {
        fprintf(
            stderr,
            "ERROR: failed to compile the shader source file `%s`\n",
            file_path
        );
        return 0;
    }

    return 1;
}

static int create_program(
    GLuint program, const char* vert_file_path, const char* frag_file_path
) {
    GLuint vert_shader = 0;
    GLuint frag_shader = 0;

    int is_compiled = 1;

    is_compiled &= compile_program_file(
        vert_file_path, GL_VERTEX_SHADER, &vert_shader
    );
    glAttachShader(program, vert_shader);

    is_compiled &= compile_program_file(
        frag_file_path, GL_FRAGMENT_SHADER, &frag_shader
    );
    glAttachShader(program, frag_shader);

    if (!is_compiled) {
        fprintf(stderr, "ERROR: failed to compile some shader files\n");
        return 0;
    }

    glLinkProgram(program);
    glDetachShader(program, vert_shader);
    glDetachShader(program, frag_shader);

    GLint is_linked;
    glGetProgramiv(program, GL_LINK_STATUS, &is_linked);
    if (is_linked != GL_TRUE) {
        GLchar message[1024];
        GLsizei message_size = 0;
        glGetProgramInfoLog(
            program, sizeof(message), &message_size, message
        );
        fprintf(stderr, "ERROR: failed to link the program %s\n", message);
        return 0;
    }

    return 1;
}

static int init_all_programs() {
    int ok = 1;

    PRIMITIVE_PROGRAM = glCreateProgram();
    ok &= create_program(
        PRIMITIVE_PROGRAM, PRIMITIVE_VERT_SHADER, PRIMITIVE_FRAG_SHADER
    );

    return ok;
}

static int get_attrib_location(
    GLuint program, GLuint* loc, const char* name
) {
    GLuint _loc = glGetAttribLocation(program, name);
    *loc = _loc;

    if (_loc == -1) {
#ifndef IGNORE_SHADER_LOC_ERRORS
        fprintf(
            stderr,
            "ERROR: failed to get the location of the attribute `%s`\n",
            name
        );
        return 0;
#endif
    }

    return 1;
}

static int get_uniform_location(
    GLuint program, GLuint* loc, const char* name
) {
    GLuint _loc = glGetUniformLocation(program, name);
    *loc = _loc;

    if (_loc == -1) {
#ifndef IGNORE_SHADER_LOC_ERRORS
        fprintf(
            stderr,
            "ERROR: failed to get the location of the uniform `%s`\n",
            name
        );
        return 0;
#endif
    }

    return 1;
}

#define _GET_UNIFORM_LOC \
    glUseProgram(program); \
    GLuint loc; \
    if (!get_uniform_location(program, &loc, name)) { \
        return 0; \
    }

static int set_uniform_1i(GLuint program, const char* name, GLint val) {
    _GET_UNIFORM_LOC
    glUniform1i(loc, val);
    return 1;
}

static int set_uniform_1f(GLuint program, const char* name, GLfloat val) {
    _GET_UNIFORM_LOC
    glUniform1f(loc, val);
    return 1;
}

static int set_uniform_2fv(
    GLuint program, const char* name, GLfloat* data, int n_values
) {
    _GET_UNIFORM_LOC
    glUniform2fv(loc, n_values, data);
    return 1;
}

static int set_uniform_3fv(
    GLuint program, const char* name, GLfloat* data, int n_values
) {
    _GET_UNIFORM_LOC
    glUniform3fv(loc, n_values, data);
    return 1;
}

static int set_uniform_4fv(
    GLuint program, const char* name, GLfloat* data, int n_values
) {
    _GET_UNIFORM_LOC
    glUniform4fv(loc, n_values, data);
    return 1;
}

static GLuint DUMMY_VAO;
static const int N_POLYGONS_IN_CIRCLE = 32;

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

    set_uniform_camera(program, WORLD.transformations[WORLD.camera]);
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

void init_renderer(void) {
    glCreateVertexArrays(1, &DUMMY_VAO);
    init_all_programs();
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

        Transformation transformation = WORLD.transformations[entity];
        Primitive primitive = WORLD.primitives[entity];
        Material material = WORLD.materials[entity];

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

    // -------------------------------------------------------------------
    // Render debug and editor-related primitives
    if (DEBUG.shading.kinematics) {
        render_debug_kinematics();
    }
    if (DEBUG.shading.collisions) {
        render_debug_collisions();
    }
    if (DEBUG.shading.visions) {
        render_debug_visions();
    }
    if (DEBUG.shading.player) {
        render_debug_player();
    }

    if (!DEBUG.is_playing) {
        render_cursor_picking();
    }

    for (int i = 0; i < DEBUG.n_primitives; ++i) {
        DebugPrimitive p = DEBUG.primitives[i];
        RenderCall render_call = prepare_primitive_render_call(
            p.transformation, p.primitive, init_material(p.color)
        );
        execute_render_call(render_call, p.fill_type);
    }

    DEBUG.n_primitives = 0;
}
