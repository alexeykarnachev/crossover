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

typedef struct GBuffer {
    GLuint fbo;
    GLuint rbo;
    GLuint world_pos_tex;
    GLuint normals_tex;
    GLuint diffuse_tex;
} GBuffer;

typedef struct LightMaskBuffer {
    GLuint fbo;
    GLuint light_mask_tex;
} LightMaskBuffer;

extern GLuint POLYGON_VAO;
extern GLuint POLYGON_VBO;
extern GBuffer GBUFFER;
extern LightMaskBuffer LIGHT_MASK_BUFFER;
extern GLuint PRIMITIVE_PROGRAM;
extern GLuint COLOR_PROGRAM;
extern GLuint LIGHT_MASK_PROGRAM;

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
