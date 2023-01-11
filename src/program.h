#pragma once

extern GLuint CIRCLE_PROGRAM;

int create_all_programs();
int get_attrib_location(GLuint program, GLuint* loc, const char* name);
int get_uniform_location(GLuint program, GLuint* loc, const char* name);
int set_uniform_1i(GLuint program, const char* name, GLint val);
int set_uniform_1f(GLuint program, const char* name, GLfloat val);
int set_uniform_1fv(
    GLuint program, const char* name, GLfloat* data, int n_values
);
