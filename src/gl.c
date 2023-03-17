#include "gl.h"
#include <glad/glad.h>

#include "app.h"
#include "collision.h"
#include "component.h"
#include "debug.h"
#include "editor.h"
#include "math.h"
#include "scene.h"
#include "system.h"
#include "utils.h"
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

int set_attrib(
    GLuint program, const char* name, int n_components, GLuint type
) {
    glUseProgram(program);
    GLuint loc;
    if (!get_attrib_location(program, &loc, name)) {
        return 0;
    }
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, n_components, type, GL_FALSE, 0, 0);
}

static GLuint POLYGON_VAO;
static GLuint POLYGON_VBO;
static const int N_POLYGONS_IN_CIRCLE = 32;

static void set_uniform_camera(GLuint program, Transformation camera) {
    CameraFrustum frustum = get_camera_frustum();
    Vec2 view_size = sub(frustum.top_right, frustum.bot_left);
    set_uniform_2fv(
        program, "camera.position", (float*)&camera.curr_position, 1
    );
    set_uniform_1f(program, "camera.orientation", camera.curr_orientation);
    set_uniform_1f(program, "camera.view_width", view_size.x);
    set_uniform_1f(program, "camera.view_height", view_size.y);
}

static void set_uniform_circle(
    GLuint program, Transformation transformation, Circle circle
) {
    set_uniform_2fv(
        program,
        "circle.position",
        (float*)&transformation.curr_position,
        1
    );
    set_uniform_1f(program, "circle.radius", circle.radius);
    set_uniform_1i(program, "circle.n_polygons", N_POLYGONS_IN_CIRCLE);
}

typedef struct RenderCall {
    int n_vertices;
    GLuint draw_mode;
} RenderCall;

static RenderCall prepare_primitive_render_call(
    Transformation transformation,
    Primitive primitive,
    Material material,
    float render_layer
) {
    if (material.type != PLAIN_COLOR_MATERIAL) {
        fprintf(
            stderr,
            "ERROR: Can render only material of type "
            "PLAIN_COLOR_MATERIAL. Another types need to be implemented\n"
        );
        exit(1);
    }

    GLuint program = PRIMITIVE_PROGRAM;
    glUseProgram(program);
    PrimitiveType type = primitive.type;

    set_uniform_camera(program, SCENE.transformations[SCENE.camera]);
    set_uniform_1i(program, "type", type);

    float* diffuse_color = (float*)&material.m.plain_color.diffuse_color;
    set_uniform_3fv(program, "diffuse_color", diffuse_color, 1);
    set_uniform_1f(program, "render_layer", render_layer);

    GLuint draw_mode = GL_TRIANGLE_FAN;
    int n_vertices;
    if (type == CIRCLE_PRIMITIVE) {
        set_uniform_circle(program, transformation, primitive.p.circle);
        n_vertices = N_POLYGONS_IN_CIRCLE + 2;
    } else {
        Vec2 vertices[MAX_N_POLYGON_VERTICES];
        n_vertices = get_primitive_fan_vertices(primitive, vertices);
        apply_transformation(vertices, n_vertices, transformation);
        glBufferSubData(
            GL_ARRAY_BUFFER, 0, n_vertices * sizeof(Vec2), (float*)vertices
        );
    }

    glBindVertexArray(POLYGON_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, POLYGON_VBO);
    set_attrib(program, "world_pos", 2, GL_FLOAT);

    RenderCall render_call;
    render_call.n_vertices = n_vertices;
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
    glDrawArrays(render_call.draw_mode, 0, render_call.n_vertices);
}

static void init_polygon_vao() {
    glCreateVertexArrays(1, &POLYGON_VAO);
    glBindVertexArray(POLYGON_VAO);

    glGenBuffers(1, &POLYGON_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, POLYGON_VBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(float) * 2 * MAX_N_POLYGON_VERTICES,
        NULL,
        GL_DYNAMIC_DRAW
    );

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void init_renderer(void) {
    init_polygon_vao();
    init_all_programs();
}

void render_scene(float dt) {
    // -------------------------------------------------------------------
    // Render primitives
    glDisable(GL_CULL_FACE);
    glViewport(0, 0, APP.window_width, APP.window_height);

    int required_component = TRANSFORMATION_COMPONENT | PRIMITIVE_COMPONENT
                             | MATERIAL_COMPONENT | RENDER_LAYER_COMPONENT;
    for (int entity = 0; entity < SCENE.n_entities; ++entity) {
        if (!check_if_entity_has_component(entity, required_component)) {
            continue;
        }

        Transformation transformation = SCENE.transformations[entity];
        Primitive primitive = SCENE.primitives[entity];
        Material material = SCENE.materials[entity];
        float render_layer = SCENE.render_layers[entity];

        RenderCall render_call = prepare_primitive_render_call(
            transformation, primitive, material, render_layer
        );

        if (DEBUG.shading.materials) {
            execute_render_call(render_call, FILL);
        }

        if (DEBUG.shading.wireframe) {
            execute_render_call(render_call, LINE);
        }
    }

    render_bullets(dt);

    // -------------------------------------------------------------------
    // Render debug and editor-related primitives
    if (!EDITOR.is_playing) {
        render_entity_handles();
        render_colliders();
    }

    if (DEBUG.shading.visions) {
        render_debug_visions();
    }
    if (DEBUG.shading.grid) {
        render_debug_grid(DEBUG.shading.grid_tile_size);
    }

    for (int i = 0; i < DEBUG.n_primitives; ++i) {
        DebugPrimitive dp = DEBUG.primitives[i];
        RenderCall render_call = prepare_primitive_render_call(
            dp.transformation,
            dp.primitive,
            init_plain_color_material(dp.color),
            dp.render_layer
        );
        execute_render_call(render_call, dp.fill_type);
    }

    DEBUG.n_primitives = 0;
}
