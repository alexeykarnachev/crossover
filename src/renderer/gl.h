#pragma once
#include <glad/glad.h>

#define GL_CHECK_ERRORS() \
    do { \
        GLenum err; \
        while ((err = glGetError()) != GL_NO_ERROR) { \
            fprintf( \
                stderr, \
                "OpenGL error in %s:%d: %d\n", \
                __FILE__, \
                __LINE__, \
                err \
            ); \
            exit(1); \
        } \
    } while (0)

int init_program(
    GLuint program, const char* vert_file_path, const char* frag_file_path
);
int init_texture_2d(
    GLuint* tex,
    void* data,
    int level,
    int width,
    int height,
    int internal_format,
    int format,
    int type,
    int filter
);

int set_attrib(
    GLuint program,
    const char* name,
    GLint n_elems,
    GLenum type,
    int offset_n_bytes
);

int set_uniform_1i(GLuint program, const char* name, GLint val);
int set_uniform_1ui(GLuint program, const char* name, GLuint val);
int set_uniform_2iv(
    GLuint program, const char* name, GLint* data, int n_values
);
int set_uniform_4iv(
    GLuint program, const char* name, GLint* data, int n_values
);
int set_uniform_1f(GLuint program, const char* name, GLfloat val);
int set_uniform_2fv(
    GLuint program, const char* name, GLfloat* data, int n_values
);
int set_uniform_3fv(
    GLuint program, const char* name, GLfloat* data, int n_values
);
int set_uniform_4fv(
    GLuint program, const char* name, GLfloat* data, int n_values
);
int get_attrib_location(GLuint program, GLuint* loc, const char* name);
int get_uniform_location(GLuint program, GLuint* loc, const char* name);
