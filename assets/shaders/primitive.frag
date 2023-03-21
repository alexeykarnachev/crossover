#version 460 core

struct ColorMaterial {
    vec3 color;
};

struct BrickMaterial {
    vec3 color;
    vec2 brick_size;
    vec2 joint_size;
    vec4 thickness;
    int is_smooth;
};

// struct StoneMaterial {
// };

in vec3 fs_world_pos;
in vec2 fs_uv_pos;

uniform float orientation;
uniform vec2 uv_size;
uniform int material_type;
uniform ColorMaterial color_material;
uniform BrickMaterial brick_material;
// uniform StoneMaterial stone_material;

layout(location=0) out vec3 world_pos_tex;
layout(location=1) out vec3 normals_tex;
layout(location=2) out vec4 diffuse_tex;

vec2 rotate2d(vec2 p, float angle) {
    mat2 rotation = mat2(cos(angle), -sin(angle), sin(angle), cos(angle));
    p = rotation * p;
    return p;
}

float min4(float a, float b, float c, float d) {
    return min(min(min(a, b), c), d);
}

void apply_brick_material(out vec3 diffuse_color, out vec3 normal) {
    vec3 color = brick_material.color;
    vec2 brick_size = brick_material.brick_size;
    vec2 joint_size = brick_material.joint_size;
    vec4 thickness = brick_material.thickness;

    vec2 brick_uv_size = brick_size / uv_size;
    vec2 joint_uv_size = joint_size / uv_size;
    vec4 thickness_uv_size = vec4(
        thickness.x / uv_size.x,
        thickness.y / uv_size.y,
        thickness.z / uv_size.x,
        thickness.w / uv_size.y
    );
    
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

    vec2 east = vec2(-1.0, 0.0);
    vec2 north = vec2(0.0, 1.0);
    vec2 west = vec2(1.0, 0.0);
    vec2 south = vec2(0.0, -1.0);
    vec2 up = vec2(0.0, 0.0);

    float west_k = 1.0 - min(1.0, fs_uv_pos.x / thickness_uv_size.x);
    float north_k = 1.0 - min(1.0, (1.0 - fs_uv_pos.y) / thickness_uv_size.y);
    float east_k = 1.0 - min(1.0, (1.0 - fs_uv_pos.x) / thickness_uv_size.z);
    float south_k = 1.0 - min(1.0, fs_uv_pos.y / thickness_uv_size.w);
    float up_k = 1.0 - min4(west_k, north_k, east_k, south_k);

    vec2 normal2d =
        rotate2d(west_k * west, orientation)
        + rotate2d(north_k * north, orientation)
        + rotate2d(east_k * east, orientation)
        + rotate2d(south_k * south, orientation);

    normal = normalize(vec3(normal2d, up_k));
}

void apply_stone_material(out vec3 diffuse_color, out vec3 normal) {
    diffuse_color = vec3(fs_uv_pos, 0.0);
    normal = vec3(0.0, 1.0, 0.0);
}

void main(void) {
    vec3 diffuse_color = vec3(0.0);
    vec3 normal = vec3(0.0);
    switch (material_type) {
        case 0:
            diffuse_color = color_material.color;
            break;
        case 1:
            apply_brick_material(diffuse_color, normal);
            break;
        case 2:
            apply_stone_material(diffuse_color, normal);
        default:
            break;
    }
    world_pos_tex = fs_world_pos;
    normals_tex = normal;
    diffuse_tex = vec4(diffuse_color, 1.0);
}
