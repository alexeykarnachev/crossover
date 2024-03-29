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

static void set_material_uniform(
    GLuint program, Material material, int idx
) {
    static char name[128];
    sprintf(name, "material_shape.materials[%d].type", idx);
    MaterialType type = material.type;
    set_uniform_1i(program, name, type);

    float* color = (float*)&material.color;
    switch (type) {
        case COLOR_MATERIAL: {
            sprintf(name, "material_shape.materials[%d].color", idx);
            set_uniform_3fv(program, name, color, 1);
            break;
        }
        case BRICK_MATERIAL: {
            sprintf(name, "material_shape.materials[%d].color", idx);
            set_uniform_3fv(program, name, color, 1);

            float* shear = (float*)&material.shear;
            sprintf(name, "material_shape.materials[%d].shear", idx);
            set_uniform_2fv(program, name, shear, 1);

            float* brick_size = (float*)&material.brick_size;
            sprintf(name, "material_shape.materials[%d].brick_size", idx);
            set_uniform_2fv(program, name, brick_size, 1);

            float* joint_size = (float*)&material.joint_size;
            sprintf(name, "material_shape.materials[%d].joint_size", idx);
            set_uniform_2fv(program, name, joint_size, 1);

            float* offset = (float*)&material.offset;
            sprintf(name, "material_shape.materials[%d].offset", idx);
            set_uniform_2fv(program, name, offset, 1);

            int* mirror = (int*)&material.mirror;
            sprintf(name, "material_shape.materials[%d].mirror", idx);
            set_uniform_2iv(program, name, mirror, 1);

            int* orientation = (int*)&material.orientation;
            sprintf(name, "material_shape.materials[%d].orientation", idx);
            set_uniform_2iv(program, name, orientation, 1);

            int* smooth_joint = (int*)&material.smooth_joint;
            sprintf(
                name, "material_shape.materials[%d].smooth_joint", idx
            );
            set_uniform_2iv(program, name, smooth_joint, 1);

            break;
        }
    }
}

static void set_material_shape_uniform(
    GLuint program, MaterialShape material_shape
) {
    set_uniform_1i(program, "material_shape.type", material_shape.type);

    switch (material_shape.type) {
        case PLANE_MATERIAL_SHAPE: {
            Material material = material_shape.materials[0];
            set_material_uniform(program, material, 0);
            break;
        }
        case CUBE_MATERIAL_SHAPE: {
            set_uniform_4fv(
                program,
                "material_shape.side_sizes",
                material_shape.side_sizes,
                1
            );
            for (int i = 0; i < 5; ++i) {
                Material material = material_shape.materials[i];
                set_material_uniform(program, material, i);
            }
            break;
        }
        default:
            break;
    }
}

static RenderCall prepare_primitive_render_call(
    GLuint program,
    Transformation transformation,
    Primitive primitive,
    MaterialShape material_shape,
    float render_layer
) {
    set_uniform_camera(program, SCENE.transformations[SCENE.camera]);
    set_uniform_1i(program, "primitive_type", primitive.type);
    set_uniform_1f(program, "render_layer", render_layer);
    set_uniform_1f(program, "elevation", transformation.elevation);
    GLuint draw_mode = GL_TRIANGLE_FAN;

    set_material_shape_uniform(program, material_shape);

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
    // -------------------------------------------------------------------
    // Collect lights from scene
    static Light lights[MAX_N_LIGHTS];
    static Primitive light_vision_primitives[MAX_N_LIGHTS];
    static Transformation light_transformations[MAX_N_LIGHTS];
    int n_lights = 0;
    for (int entity = 0; entity < SCENE.n_entities; ++entity) {
        if (n_lights == MAX_N_LIGHTS) {
            break;
        }

        if (!check_if_entity_has_component(
                entity, LIGHT_COMPONENT | TRANSFORMATION_COMPONENT
            )) {
            continue;
        }

        Transformation transformation = SCENE.transformations[entity];
        Light light = SCENE.lights[entity];

        lights[n_lights] = light;
        light_vision_primitives[n_lights] = init_circle_primitive(
            light.radius
        );
        light_transformations[n_lights] = transformation;

        n_lights += 1;
    }

    // -------------------------------------------------------------------
    // Render primitives
    glBindFramebuffer(GL_FRAMEBUFFER, GBUFFER.fbo);
    glViewport(0, 0, GBUFFER_WIDTH, GBUFFER_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
    glUseProgram(PRIMITIVE_PROGRAM);

    int required_component = TRANSFORMATION_COMPONENT | PRIMITIVE_COMPONENT
                             | MATERIAL_SHAPE_COMPONENT
                             | RENDER_LAYER_COMPONENT;
    for (int entity = 0; entity < SCENE.n_entities; ++entity) {
        if (!check_if_entity_has_component(entity, required_component)) {
            continue;
        }

        Transformation transformation = SCENE.transformations[entity];
        Primitive primitive = SCENE.primitives[entity];
        MaterialShape material_shape = SCENE.material_shapes[entity];
        float render_layer = SCENE.render_layers[entity];

        RenderCall render_call = prepare_primitive_render_call(
            PRIMITIVE_PROGRAM,
            transformation,
            primitive,
            material_shape,
            render_layer
        );

        if (DEBUG.shading.materials) {
            execute_render_call(render_call, FILL);
        }

        if (DEBUG.shading.wireframe) {
            execute_render_call(render_call, LINE);
        }
    }

    // -------------------------------------------------------------------
    // Render bullets
    glUseProgram(PRIMITIVE_PROGRAM);
    render_bullets(dt);

    // -------------------------------------------------------------------
    // Render debug and editor-related primitives
    glUseProgram(PRIMITIVE_PROGRAM);

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
            PRIMITIVE_PROGRAM,
            dp.transformation,
            dp.primitive,
            init_plane_material_shape(init_color_material(dp.color)),
            dp.render_layer
        );
        execute_render_call(render_call, dp.fill_type);
    }

    DEBUG.n_primitives = 0;
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // -------------------------------------------------------------------
    // Render light mask
    glBindFramebuffer(GL_FRAMEBUFFER, LIGHT_MASK_BUFFER.fbo);
    glViewport(0, 0, GBUFFER_WIDTH, GBUFFER_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
    glUseProgram(LIGHT_MASK_PROGRAM);

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    for (int i = 0; i < n_lights; ++i) {
        RenderCall render_call = prepare_primitive_render_call(
            LIGHT_MASK_PROGRAM,
            light_transformations[i],
            light_vision_primitives[i],
            init_default_plane_material_shape(),
            0.0
        );
        set_uniform_1i(LIGHT_MASK_PROGRAM, "light_idx", i);
        execute_render_call(render_call, FILL);
    }
    glDisable(GL_BLEND);

    // -------------------------------------------------------------------
    // Blit color to the output frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, APP.window_width, APP.window_height);
    GLuint program = COLOR_PROGRAM;
    glUseProgram(program);

    // TODO: Maybe move lights setting to a separate function

    // -------------------------------------------------------------------
    // Set up lights
    static char color_name[32];
    static char attenuation_name[32];
    static char is_dir_name[32];
    static char vec_name[32];
    static char power_name[32];

    if (DEBUG.shading.lights == 1) {
        for (int i = 0; i < n_lights; ++i) {
            Transformation transformation = light_transformations[i];
            Light light = lights[i];

            sprintf(color_name, "lights[%d].color", i);
            sprintf(attenuation_name, "lights[%d].attenuation", i);
            sprintf(is_dir_name, "lights[%d].is_dir", i);
            sprintf(vec_name, "lights[%d].vec", i);
            sprintf(power_name, "lights[%d].power", i);

            set_uniform_3fv(program, color_name, (float*)&light.color, 1);
            set_uniform_3fv(
                program, attenuation_name, (float*)&light.attenuation, 1
            );
            set_uniform_1f(program, power_name, light.power);
            set_uniform_1i(program, is_dir_name, light.is_dir);

            if (light.is_dir == 1) {
                set_uniform_3fv(
                    program, vec_name, (float*)&light.direction, 1
                );
            } else {
                Vec3 position = vec3(
                    transformation.curr_position.x,
                    transformation.curr_position.y,
                    transformation.elevation
                );
                set_uniform_3fv(program, vec_name, (float*)&position, 1);
            }
        }
    }

    set_uniform_1i(program, "enable_lights", DEBUG.shading.lights);
    set_uniform_1i(program, "n_lights", n_lights);

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

    set_uniform_1i(program, "light_mask_tex", 3);
    glActiveTexture(GL_TEXTURE0 + 3);
    glBindTexture(GL_TEXTURE_2D, LIGHT_MASK_BUFFER.light_mask_tex);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
