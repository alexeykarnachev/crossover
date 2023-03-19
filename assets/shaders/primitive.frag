#version 460 core

struct ColorMaterial {
    vec3 color;
};

struct BrickMaterial {
    vec3 color;
    vec2 brick_size;
    vec2 joint_size;
    int is_smooth;
};

// struct StoneMaterial {
// };

in vec3 fs_world_pos;
in vec2 fs_uv_pos;

uniform vec2 uv_size;
uniform int material_type;
uniform ColorMaterial color_material;
uniform BrickMaterial brick_material;
// uniform StoneMaterial stone_material;

layout(location=0) out vec3 world_pos_tex;
layout(location=1) out vec4 diffuse_tex;

void apply_brick_material(out vec3 diffuse_color, out vec3 normals) {
    vec3 color = brick_material.color;
    vec2 brick_size = brick_material.brick_size;
    vec2 joint_size = brick_material.joint_size;

    vec2 brick_uv_size = brick_size / uv_size;
    vec2 joint_uv_size = joint_size / uv_size;
    
    vec2 n_bricks = fs_uv_pos / brick_uv_size;
    vec2 brick_fract = fract(n_bricks);

    if (int(n_bricks.y) % 2 == 0) {
        brick_fract.x = fract(brick_fract.x + 0.5);
    }
    brick_fract.x = min(brick_fract.x, 1.0 - brick_fract.x);
    brick_fract.y = min(brick_fract.y, 1.0 - brick_fract.y);

    float joint_x;
    float joint_y;
    float joint_edge_x = joint_uv_size.x / brick_uv_size.x;
    float joint_edge_y = joint_uv_size.y / brick_uv_size.y;
    if (brick_material.is_smooth == 1) {
        joint_x = smoothstep(0.0, joint_edge_x, brick_fract.x);
        joint_y = smoothstep(0.0, joint_edge_y, brick_fract.y);
    } else {
        joint_x = step(joint_edge_x, brick_fract.x);
        joint_y = step(joint_edge_y, brick_fract.y);
    }
    color *= min(joint_x, joint_y);

    diffuse_color = color;
    normals = vec3(0.0, 1.0, 0.0);
}

void apply_stone_material(out vec3 diffuse_color, out vec3 normals) {
    diffuse_color = vec3(fs_uv_pos, 0.0);
    normals = vec3(0.0, 1.0, 0.0);
}

void main(void) {
    vec3 diffuse_color = vec3(0.0);
    vec3 normals = vec3(0.0);
    switch (material_type) {
        case 0:
            diffuse_color = color_material.color;
            break;
        case 1:
            apply_brick_material(diffuse_color, normals);
            break;
        case 2:
            apply_stone_material(diffuse_color, normals);
        default:
            break;
    }
    world_pos_tex = fs_world_pos;
    diffuse_tex = vec4(diffuse_color, 1.0);
}
