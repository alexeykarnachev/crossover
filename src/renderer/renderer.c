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

void render_scene(float dt) {
    update_buffers();

    glBindFramebuffer(GL_FRAMEBUFFER, GBUFFER.fbo);
    glViewport(0, 0, GBUFFER_WIDTH, GBUFFER_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
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

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0);
    glEnable(GL_CULL_FACE);
}
