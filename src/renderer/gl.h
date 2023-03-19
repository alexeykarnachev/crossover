#pragma once
#include <glad/glad.h>

extern GLuint POLYGON_VAO;
extern GLuint POLYGON_VBO;
extern GLuint PRIMITIVE_PROGRAM;

int set_attrib(
    GLuint program,
    const char* name,
    GLint n_elems,
    GLenum type,
    int offset_n_bytes
);

int set_uniform_1i(GLuint program, const char* name, GLint val);
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
