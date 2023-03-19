#include "gl.h"

#include "../const.h"
#include "../math.h"
#include "../renderer.h"
#include "../utils.h"
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GLuint POLYGON_VAO;
GLuint POLYGON_VBO;
GLuint PRIMITIVE_PROGRAM;
GBuffer GBUFFER;

static void init_polygon_vao(void) {
    glCreateVertexArrays(1, &POLYGON_VAO);
    glBindVertexArray(POLYGON_VAO);

    glGenBuffers(1, &POLYGON_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, POLYGON_VBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(Vec2) * 2 * MAX_N_POLYGON_VERTICES,
        NULL,
        GL_DYNAMIC_DRAW
    );
    GL_CHECK_ERRORS();

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    GL_CHECK_ERRORS();
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

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

static int init_all_programs(void) {
    int ok = 1;

    PRIMITIVE_PROGRAM = glCreateProgram();
    ok &= create_program(
        PRIMITIVE_PROGRAM, PRIMITIVE_VERT_SHADER, PRIMITIVE_FRAG_SHADER
    );

    return ok;
}

static int create_texture_2d(
    GLuint* tex,
    void* data,
    size_t level,
    size_t width,
    size_t height,
    size_t internal_format,
    size_t format,
    size_t type,
    int filter
) {
    glCreateTextures(GL_TEXTURE_2D, 1, tex);
    glBindTexture(GL_TEXTURE_2D, *tex);

    glTexImage2D(
        GL_TEXTURE_2D,
        level,
        internal_format,
        width,
        height,
        0,
        format,
        type,
        data
    );
    GL_CHECK_ERRORS();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glBindTexture(GL_TEXTURE_2D, 0);

    return 1;
}

static int init_gbuffer(void) {
    glGenFramebuffers(1, &GBUFFER.fbo);
    glGenRenderbuffers(1, &GBUFFER.rbo);

    create_texture_2d(
        &GBUFFER.world_pos_tex,
        NULL,
        0,
        GBUFFER_WIDTH,
        GBUFFER_HEIGHT,
        GL_RGB32F,
        GL_RGB,
        GL_FLOAT,
        GL_NEAREST
    );

    create_texture_2d(
        &GBUFFER.diffuse_tex,
        NULL,
        0,
        GBUFFER_WIDTH,
        GBUFFER_HEIGHT,
        GL_RGBA32F,
        GL_RGBA,
        GL_FLOAT,
        GL_NEAREST
    );

    glBindRenderbuffer(GL_RENDERBUFFER, GBUFFER.rbo);
    glRenderbufferStorage(
        GL_RENDERBUFFER, GL_DEPTH_COMPONENT, GBUFFER_WIDTH, GBUFFER_HEIGHT
    );
    GL_CHECK_ERRORS();

    glBindFramebuffer(GL_FRAMEBUFFER, GBUFFER.fbo);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0 + 0,
        GL_TEXTURE_2D,
        GBUFFER.world_pos_tex,
        0
    );
    GL_CHECK_ERRORS();

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0 + 2,
        GL_TEXTURE_2D,
        GBUFFER.diffuse_tex,
        0
    );
    GL_CHECK_ERRORS();

    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, GBUFFER.rbo
    );
    GL_CHECK_ERRORS();

    GLuint buffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, buffers);
    GL_CHECK_ERRORS();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return 1;
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

void init_gl(void) {
    init_polygon_vao();
    init_all_programs();
    init_gbuffer();
}

#define _GET_UNIFORM_LOC \
    glUseProgram(program); \
    GLuint loc; \
    if (!get_uniform_location(program, &loc, name)) { \
        return 0; \
    }

int set_attrib(
    GLuint program,
    const char* name,
    GLint n_elems,
    GLenum type,
    int offset_n_bytes
) {
    glUseProgram(program);
    GLuint loc;
    if (!get_attrib_location(program, &loc, name)) {
        return 0;
    }

    void* offset = (void*)(uint64_t)offset_n_bytes;
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, n_elems, type, GL_FALSE, 0, offset);
    return 1;
}

int set_uniform_1i(GLuint program, const char* name, GLint val) {
    _GET_UNIFORM_LOC
    glUniform1i(loc, val);
    return 1;
}

int set_uniform_1f(GLuint program, const char* name, GLfloat val) {
    _GET_UNIFORM_LOC
    glUniform1f(loc, val);
    return 1;
}

int set_uniform_2fv(
    GLuint program, const char* name, GLfloat* data, int n_values
) {
    _GET_UNIFORM_LOC
    glUniform2fv(loc, n_values, data);
    return 1;
}

int set_uniform_3fv(
    GLuint program, const char* name, GLfloat* data, int n_values
) {
    _GET_UNIFORM_LOC
    glUniform3fv(loc, n_values, data);
    return 1;
}

int set_uniform_4fv(
    GLuint program, const char* name, GLfloat* data, int n_values
) {
    _GET_UNIFORM_LOC
    glUniform4fv(loc, n_values, data);
    return 1;
}
