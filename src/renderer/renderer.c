#include <glad/glad.h>

#include "../app.h"
#include "../collision.h"
#include "../component.h"
#include "../debug.h"
#include "../editor.h"
#include "../math.h"
#include "../renderer.h"
#include "../scene.h"
#include "gl.h"
#include <stdlib.h>
#include <string.h>

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

static GBuffer GBUFFER;
static LightMaskBuffer LIGHT_MASK_BUFFER;

static GLuint CIRCLE_PROGRAM;
static GLuint DEBUG_SCREEN_RECT_PROGRAM;

static GLuint CIRCLE_VAO;
static GLuint CIRCLE_POS_VBO;
static GLuint CIRCLE_GEOMETRY_VBO;
static GLuint CIRCLE_RENDER_LAYER_VBO;

static int N_CIRCLES = 0;
static float CIRCLE_POS_DATA[MAX_N_POLYGON_VERTICES * 2];
static float CIRCLE_GEOMETRY_DATA[MAX_N_ENTITIES * 4];
static float CIRCLE_RENDER_LAYER_DATA[MAX_N_ENTITIES];

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

static void update_buffers(void) {
    N_CIRCLES = 0;

    int required_component = TRANSFORMATION_COMPONENT | PRIMITIVE_COMPONENT
                             | MATERIAL_SHAPE_COMPONENT
                             | RENDER_LAYER_COMPONENT;
    for (int entity = 0; entity < SCENE.n_entities; ++entity) {
        if (!check_if_entity_has_component(entity, required_component)) {
            continue;
        }

        Primitive primitive = SCENE.primitives[entity];
        Transformation transformation = SCENE.transformations[entity];
        MaterialShape material_shape = SCENE.material_shapes[entity];
        float render_layer = SCENE.render_layers[entity];

        switch (primitive.type) {
            case CIRCLE_PRIMITIVE: {
                CIRCLE_GEOMETRY_DATA[N_CIRCLES * 4 + 0]
                    = transformation.curr_position.x;
                CIRCLE_GEOMETRY_DATA[N_CIRCLES * 4 + 1]
                    = transformation.curr_position.y;
                CIRCLE_GEOMETRY_DATA[N_CIRCLES * 4 + 2]
                    = transformation.elevation;
                CIRCLE_GEOMETRY_DATA[N_CIRCLES * 4 + 3]
                    = primitive.p.circle.radius;
                CIRCLE_RENDER_LAYER_DATA[N_CIRCLES] = render_layer;
                N_CIRCLES += 1;
                break;
            }
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, CIRCLE_GEOMETRY_VBO);
    glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        N_CIRCLES * sizeof(float) * 4,
        CIRCLE_GEOMETRY_DATA
    );

    glBindBuffer(GL_ARRAY_BUFFER, CIRCLE_RENDER_LAYER_VBO);
    glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        N_CIRCLES * sizeof(float),
        CIRCLE_RENDER_LAYER_DATA
    );
}

static void render_circles(void) {
    glBindVertexArray(CIRCLE_VAO);
    glUseProgram(CIRCLE_PROGRAM);

    set_uniform_camera(
        CIRCLE_PROGRAM, SCENE.transformations[SCENE.camera]
    );

    glBindBuffer(GL_ARRAY_BUFFER, CIRCLE_POS_VBO);
    set_attrib(CIRCLE_PROGRAM, "vs_pos", 2, GL_FLOAT, 0);
    glBindBuffer(GL_ARRAY_BUFFER, CIRCLE_GEOMETRY_VBO);
    set_attrib(CIRCLE_PROGRAM, "vs_geometry", 4, GL_FLOAT, 0);
    glBindBuffer(GL_ARRAY_BUFFER, CIRCLE_RENDER_LAYER_VBO);
    set_attrib(CIRCLE_PROGRAM, "vs_render_layer", 1, GL_FLOAT, 0);

    glVertexAttribDivisor(0, 0);
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);

    glDrawArraysInstanced(
        GL_TRIANGLE_FAN, 0, MAX_N_POLYGON_VERTICES, N_CIRCLES
    );
}

void render_scene(float dt) {
    update_buffers();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, APP.window_width, APP.window_height);
    glDisable(GL_CULL_FACE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    render_circles();
}

static void init_circle_vao(void) {
    Vec2 vertices[MAX_N_POLYGON_VERTICES];
    get_unit_circle_fan_vertices(vertices, MAX_N_POLYGON_VERTICES);

    glCreateVertexArrays(1, &CIRCLE_VAO);
    glBindVertexArray(CIRCLE_VAO);

    // -------------------------------------------------------------------
    // Circle vertex data (positions which are common for all instances)
    glGenBuffers(1, &CIRCLE_POS_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, CIRCLE_POS_VBO);
    glBufferData(
        GL_ARRAY_BUFFER, sizeof(vertices), (float*)vertices, GL_STATIC_DRAW
    );
    GL_CHECK_ERRORS();

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribDivisor(0, 0);
    GL_CHECK_ERRORS();

    // -------------------------------------------------------------------
    // Circle instance data: vec4(x, y, elevation, radius)
    glGenBuffers(1, &CIRCLE_GEOMETRY_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, CIRCLE_GEOMETRY_VBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        MAX_N_ENTITIES * 4 * sizeof(float),
        0,
        GL_DYNAMIC_DRAW
    );
    GL_CHECK_ERRORS();

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribDivisor(1, 1);
    GL_CHECK_ERRORS();

    // -------------------------------------------------------------------
    // Circle instance data: render_layer
    glGenBuffers(1, &CIRCLE_RENDER_LAYER_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, CIRCLE_RENDER_LAYER_VBO);
    glBufferData(
        GL_ARRAY_BUFFER, MAX_N_ENTITIES * sizeof(float), 0, GL_DYNAMIC_DRAW
    );
    GL_CHECK_ERRORS();

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribDivisor(2, 1);
    GL_CHECK_ERRORS();

    // Bind default back
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static int init_all_programs(void) {
    int ok = 1;

    CIRCLE_PROGRAM = glCreateProgram();
    ok &= init_program(
        CIRCLE_PROGRAM, CIRCLE_VERT_SHADER, MATERIAL_FRAG_SHADER
    );

    DEBUG_SCREEN_RECT_PROGRAM = glCreateProgram();
    ok &= init_program(
        DEBUG_SCREEN_RECT_PROGRAM,
        SCREEN_RECT_VERT_SHADER,
        PLANE_COLOR_FRAG_SHADER
    );

    // COLOR_PROGRAM = glCreateProgram();
    // ok &= init_program(
    //     COLOR_PROGRAM, SCREEN_RECT_VERT_SHADER, COLOR_FRAG_SHADER
    // );

    // LIGHT_MASK_PROGRAM = glCreateProgram();
    // ok &= init_program(
    //     LIGHT_MASK_PROGRAM, PRIMITIVE_VERT_SHADER,
    //     LIGHT_MASK_FRAG_SHADER
    // );

    return ok;
}

static int init_gbuffer(void) {
    // TODO: Factor out textures creations (a lot of repetitions here)
    glGenFramebuffers(1, &GBUFFER.fbo);
    glGenRenderbuffers(1, &GBUFFER.rbo);

    init_texture_2d(
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

    init_texture_2d(
        &GBUFFER.normals_tex,
        NULL,
        0,
        GBUFFER_WIDTH,
        GBUFFER_HEIGHT,
        GL_RGB32F,
        GL_RGB,
        GL_FLOAT,
        GL_NEAREST
    );

    init_texture_2d(
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
        GL_COLOR_ATTACHMENT0 + 1,
        GL_TEXTURE_2D,
        GBUFFER.normals_tex,
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

    GLuint buffers[3] = {
        GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, buffers);
    GL_CHECK_ERRORS();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return 1;
}

void init_renderer(void) {
    init_all_programs();
    init_circle_vao();
    // init_gbuffer();
}
