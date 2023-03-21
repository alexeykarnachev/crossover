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
    GLuint program = PRIMITIVE_PROGRAM;
    glUseProgram(program);
    set_uniform_camera(program, SCENE.transformations[SCENE.camera]);
    set_uniform_1i(program, "primitive_type", primitive.type);
    set_uniform_1i(program, "material_type", material.type);
    set_uniform_1f(program, "render_layer", render_layer);
    GLuint draw_mode = GL_TRIANGLE_FAN;

    switch (material.type) {
        case COLOR_MATERIAL: {
            float* color = (float*)&material.m.color.color;
            set_uniform_3fv(program, "color_material.color", color, 1);
            break;
        }
        case WALL_MATERIAL: {
            float* color = (float*)&material.m.color.color;
            float* brick_size = (float*)&material.m.wall.brick_size;
            float* joint_size = (float*)&material.m.wall.joint_size;
            float* tilt = (float*)&material.m.wall.tilt;
            int* flip = (int*)&material.m.wall.flip;
            float elevation = material.m.wall.elevation;
            int smooth_joint = material.m.wall.smooth_joint;

            set_uniform_3fv(program, "wall_material.color", color, 1);
            set_uniform_2fv(
                program, "wall_material.brick_size", brick_size, 1
            );
            set_uniform_2fv(
                program, "wall_material.joint_size", joint_size, 1
            );
            set_uniform_4fv(program, "wall_material.tilt", tilt, 1);
            set_uniform_4iv(program, "wall_material.flip", flip, 1);
            set_uniform_1f(program, "wall_material.elevation", elevation);
            set_uniform_1i(
                program, "wall_material.smooth_joint", smooth_joint
            );
            break;
        }
        default:
            break;
    }

    int total_attrib_size = sizeof(Vec2);
    int n_vertices;
    Vec2 uv_size;
    if (primitive.type == CIRCLE_PRIMITIVE) {
        set_uniform_circle(program, transformation, primitive.p.circle);
        n_vertices = N_POLYGONS_IN_CIRCLE + 2;
        total_attrib_size *= n_vertices;
    } else {
        static Vec2 vertices[MAX_N_POLYGON_VERTICES];
        static Vec2 uvs[MAX_N_POLYGON_VERTICES];
        Vec2 uv_size;

        n_vertices = get_primitive_fan_vertices(primitive, vertices);
        total_attrib_size *= n_vertices;

        get_vertex_uvs(vertices, n_vertices, uvs, &uv_size);
        apply_transformation(vertices, n_vertices, transformation);

        glBufferSubData(
            GL_ARRAY_BUFFER, 0, total_attrib_size, (float*)vertices
        );
        glBufferSubData(
            GL_ARRAY_BUFFER,
            total_attrib_size,
            total_attrib_size,
            (float*)uvs
        );
    }

    glBindVertexArray(POLYGON_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, POLYGON_VBO);

    set_attrib(program, "vs_world_pos", 2, GL_FLOAT, 0);
    set_attrib(program, "vs_uv_pos", 2, GL_FLOAT, total_attrib_size);
    set_uniform_1f(
        program, "orientation", transformation.curr_orientation
    );
    set_uniform_2fv(program, "uv_size", (float*)&uv_size, 1);

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

void render_scene(float dt) {
    glBindFramebuffer(GL_FRAMEBUFFER, GBUFFER.fbo);
    glViewport(0, 0, GBUFFER_WIDTH, GBUFFER_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // -------------------------------------------------------------------
    // Render primitives
    glDisable(GL_CULL_FACE);

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
            init_color_material(dp.color),
            dp.render_layer
        );
        execute_render_call(render_call, dp.fill_type);
    }

    DEBUG.n_primitives = 0;
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // -------------------------------------------------------------------
    // Blit color to the output frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, APP.window_width, APP.window_height);
    GLuint program = COLOR_PROGRAM;
    glUseProgram(program);

    // TODO: Factor out these bindings
    set_uniform_1i(program, "world_pos_tex", 0);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, GBUFFER.world_pos_tex);

    set_uniform_1i(program, "normals_tex", 1);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, GBUFFER.normals_tex);

    set_uniform_1i(program, "diffuse_tex", 2);
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, GBUFFER.diffuse_tex);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
