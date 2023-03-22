#version 460 core

#define PI 3.14159265359

struct ColorMaterial {
    vec3 color;
};

struct WallMaterial {
    vec3 color;
    vec2 brick_size;
    vec2 joint_size;
    vec4 tilt;
    ivec4 flip;
    float elevation;
    int smooth_joint;
};

struct Wall {
    vec3 normal;
    float elevation;
    int side;
    int flip;
};

const vec3 WEST_NORMAL = vec3(-1.0, 0.0, 0.0);
const vec3 NORTH_NORMAL = vec3(0.0, 1.0, 0.0);
const vec3 EAST_NORMAL = vec3(1.0, 0.0, 0.0);
const vec3 SOUTH_NORMAL = vec3(0.0, -1.0, 0.0);
const vec3 UP_NORMAL = vec3(0.0, 0.0, 1.0);

const int WEST_SIDE = 0;
const int NORTH_SIDE = 1;
const int EAST_SIDE = 2;
const int SOUTH_SIDE = 3;
const int UP_SIDE = 4;

in vec3 fs_world_pos;
in vec2 fs_uv_pos;

uniform float orientation;
uniform vec2 uv_size;
uniform int material_type;
uniform ColorMaterial color_material;
uniform WallMaterial wall_material;

layout(location=0) out vec3 world_pos_tex;
layout(location=1) out vec3 normals_tex;
layout(location=2) out vec4 diffuse_tex;

vec2 rotate2d(vec2 p, float angle) {
    float c = cos(angle);
    float s = sin(angle);
    return vec2(c * p.x - s * p.y, s * p.x + c * p.y);
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

vec3 get_wall_color(Wall wall) {
    vec3 color = wall_material.color;
    vec2 brick_size = wall_material.brick_size;
    vec2 joint_size = wall_material.joint_size;

    vec2 brick_uv_size = brick_size / uv_size;
    vec2 joint_uv_size = joint_size / uv_size;
    vec2 uv = fs_uv_pos;

    if (wall.flip == 1) {
        if (wall.side == WEST_SIDE || wall.side == EAST_SIDE) {
            uv.y = 1.0 - uv.y;
        } else if (wall.side == NORTH_SIDE || wall.side == SOUTH_SIDE) {
            uv.x = 1.0 - uv.x;
        }
    }

    if (wall.side == WEST_SIDE) {
        uv.y -= 0.5;
        uv.x = 1.0 - uv.x;
        uv = rotate2d(uv, PI * 0.5);
        brick_uv_size = brick_size / swap2(uv_size);
        joint_uv_size = joint_size / swap2(uv_size);
    } else if (wall.side == EAST_SIDE) {
        uv.y -= 0.5;
        uv = rotate2d(uv, PI * 0.5);
        brick_uv_size = brick_size / swap2(uv_size);
        joint_uv_size = joint_size / swap2(uv_size);
    } else if (wall.side == SOUTH_SIDE) {
        uv.x -= 0.5;
        uv.y = 1.0 - uv.y;
    } else if (wall.side == NORTH_SIDE) {
        uv.x -= 0.5;
    }

    brick_uv_size *= wall.elevation;
    joint_uv_size *= wall.elevation;
    
    vec2 n_bricks = uv / brick_uv_size;
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
    if (wall_material.smooth_joint == 1) {
        joint_x = smoothstep(0.0, joint_edge_x, brick_fract.x);
        joint_y = smoothstep(0.0, joint_edge_y, brick_fract.y);
    } else {
        joint_x = step(joint_edge_x, brick_fract.x);
        joint_y = step(joint_edge_y, brick_fract.y);
    }
    color *= min(joint_x, joint_y);

    return color;
}

Wall get_wall(void) {
    vec4 tilt = wall_material.tilt;
    vec4 tilt_uv_size = vec4(
        tilt.x / uv_size.x,
        tilt.y / uv_size.y,
        tilt.z / uv_size.x,
        tilt.w / uv_size.y
    );

    float west_k = 1.0 - min(1.0, fs_uv_pos.x / tilt_uv_size.x);
    float north_k = 1.0 - min(1.0, (1.0 - fs_uv_pos.y) / tilt_uv_size.y);
    float east_k = 1.0 - min(1.0, (1.0 - fs_uv_pos.x) / tilt_uv_size.z);
    float south_k = 1.0 - min(1.0, fs_uv_pos.y / tilt_uv_size.w);

    float max_k = max4(west_k, north_k, east_k, south_k);
    vec3 normal;
    int side;
    if (max_k == 0.0) {
        normal = UP_NORMAL;
        side = UP_SIDE;
    } else if (max_k == west_k) {
        normal = vec3(rotate2d(WEST_NORMAL.xy, orientation), 0.0);
        side = WEST_SIDE;
    } else if (max_k == north_k) {
        normal = vec3(rotate2d(NORTH_NORMAL.xy, orientation), 0.0);
        side = NORTH_SIDE;
    } else if (max_k == east_k) {
        normal = vec3(rotate2d(EAST_NORMAL.xy, orientation), 0.0);
        side = EAST_SIDE;
    } else if (max_k == south_k) {
        normal = vec3(rotate2d(SOUTH_NORMAL.xy, orientation), 0.0);
        side = SOUTH_SIDE;
    }

    Wall wall;
    wall.normal = normal;
    wall.side = side;
    wall.flip = side == UP_SIDE ? 0 : wall_material.flip[side];
    wall.elevation =
        (1.0 - wall_material.elevation)
        + (1.0 - max_k) * wall_material.elevation;
    return wall;
}

void apply_wall_material(out vec3 diffuse_color, out vec3 normal) {
    Wall wall = get_wall();
    diffuse_color = get_wall_color(wall);

    normal = wall.normal;
}

void main(void) {
    vec3 diffuse_color = vec3(0.0);
    vec3 normal = vec3(0.0);
    switch (material_type) {
        case 0:
            diffuse_color = color_material.color;
            break;
        case 1:
            apply_wall_material(diffuse_color, normal);
            break;
        default:
            break;
    }
    world_pos_tex = fs_world_pos;
    normals_tex = normal;
    diffuse_tex = vec4(diffuse_color, 1.0);
}
