#version 460 core

#define PI 3.14159265359

struct Material {
    int type;

    // Common
    vec3 color;

    // Brick
    vec2 shear;
    vec2 brick_size;
    vec2 joint_size;
    vec2 offset;
    ivec2 mirror;
    ivec2 orientation;
    ivec2 smooth_joint;
};

struct MaterialShape {
    int type;

    // Common
    Material materials[5];

    // Cube
    vec4 side_sizes;
};

const int PLANE_MATERIAL_SHAPE = 0;
const int CUBE_MATERIAL_SHAPE = 1;
const int COLOR_MATERIAL = 0;
const int BRICK_MATERIAL = 1;

// West, north, east, south
const vec2 DIRECTIONS[4] = {
    vec2(-1.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 0.0),
    vec2(0.0, -1.0)
};

const int WEST_SIDE = 0;
const int NORTH_SIDE = 1;
const int EAST_SIDE = 2;
const int SOUTH_SIDE = 3;
const int UP_SIDE = 4;

in vec3 fs_world_pos;
in vec2 fs_uv_pos;

uniform float orientation;
uniform vec2 uv_size;
uniform MaterialShape material_shape;

layout(location=0) out vec3 world_pos_tex;
layout(location=1) out vec3 normals_tex;
layout(location=2) out vec4 diffuse_tex;

vec2 rotate2d(vec2 p, vec2 origin, float angle) {
    p -= origin;
    float c = cos(angle);
    float s = sin(angle);
    return vec2(c * p.x - s * p.y, s * p.x + c * p.y) + origin;
}

float min4(float a, float b, float c, float d) {
    return min(min(min(a, b), c), d);
}

float max4(float a, float b, float c, float d) {
    return max(max(max(a, b), c), d);
}

vec2 swap2(vec2 v) {
    return vec2(v.y, v.x);
}

int get_cube_side(void) {
    vec4 sizes = material_shape.side_sizes;
    vec4 uv_sizes = sizes / vec4(uv_size, uv_size);

    float west_k = 1.0 - min(1.0, fs_uv_pos.x / uv_sizes.x);
    float north_k = 1.0 - min(1.0, (1.0 - fs_uv_pos.y) / uv_sizes.y);
    float east_k = 1.0 - min(1.0, (1.0 - fs_uv_pos.x) / uv_sizes.z);
    float south_k = 1.0 - min(1.0, fs_uv_pos.y / uv_sizes.w);
    float max_k = max4(west_k, north_k, east_k, south_k);

    int side;
    if (max_k == 0.0) {
        side = UP_SIDE;
    } else if (max_k == west_k) {
        side = WEST_SIDE;
    } else if (max_k == north_k) {
        side = NORTH_SIDE;
    } else if (max_k == east_k) {
        side = EAST_SIDE;
    } else if (max_k == south_k) {
        side = SOUTH_SIDE;
    }

    return side;
}

vec3 get_cube_normal(int side) {
    if (side == UP_SIDE) {
        return vec3(0.0, 0.0, 1.0);
    }

    vec2 direction = DIRECTIONS[side];
    vec3 normal = vec3(rotate2d(direction, vec2(0.0), orientation), 0.0);
    return normal;
}

vec2 mirror(vec2 uv, ivec2 flags) {
    vec2 new_uv = uv;
    if (flags.x == 1) {
        new_uv.x = 1.0 - uv.x;
    }
    if (flags.y == 1) {
        new_uv.y = 1.0 - uv.y;
    }
    return new_uv;
}

vec3 get_brick_color(Material material) {
    vec2 uv = mirror(fs_uv_pos, material.mirror) + material.offset;
    vec2 brick_uv_size = material.brick_size / uv_size;
    vec2 joint_uv_size = material.joint_size / uv_size;

    if (material.orientation.x == 1 && int(uv.y / brick_uv_size.y) % 2 == 0) {
        uv.x += brick_uv_size.x * 0.5;
    }

    if (material.orientation.y == 1 && int(uv.x / brick_uv_size.x) % 2 == 0) {
        uv.y += brick_uv_size.y * 0.5;
    }

    uv = vec2(
        uv.x + material.shear.x * uv.y,
        uv.y + material.shear.y * uv.x
    );

    vec2 n_bricks = uv / brick_uv_size;
    vec2 brick_fract = fract(n_bricks);

    float joint_x;
    float joint_y;
    float joint_edge_x = joint_uv_size.x / brick_uv_size.x;
    float joint_edge_y = joint_uv_size.y / brick_uv_size.y;
    if (material.smooth_joint.x == 1) {
        joint_x = smoothstep(0.0, joint_edge_x, brick_fract.x);
    } else {
        joint_x = step(joint_edge_x, brick_fract.x);
    }

    if (material.smooth_joint.y == 1) {
        joint_y = smoothstep(0.0, joint_edge_y, brick_fract.y);
    } else {
        joint_y = step(joint_edge_y, brick_fract.y);
    }

    vec3 color = material.color * min(joint_x, joint_y);
    return color;
}

void main(void) {
    vec3 diffuse_color = vec3(0.0);
    vec3 normal = vec3(0.0);

    int side;
    Material material;
    switch (material_shape.type) {
        case PLANE_MATERIAL_SHAPE:
            side = UP_SIDE;
            material = material_shape.materials[0];
            break;
        case CUBE_MATERIAL_SHAPE: {
            side = get_cube_side();
            normal = get_cube_normal(side);
            material = material_shape.materials[side];
            break;
        }
    }

    switch(material.type) {
        case COLOR_MATERIAL:
            diffuse_color = material.color;
            break;
        case BRICK_MATERIAL:
            diffuse_color = get_brick_color(material);
            break;
    }

    world_pos_tex = fs_world_pos;
    normals_tex = normal;
    diffuse_tex = vec4(diffuse_color, 1.0);
}
