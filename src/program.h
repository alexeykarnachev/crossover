#pragma once

extern GLuint PRIMITIVE_PROGRAM;

int init_all_programs();
int get_attrib_location(GLuint program, GLuint* loc, const char* name);
int get_uniform_location(GLuint program, GLuint* loc, const char* name);
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
