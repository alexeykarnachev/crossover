#version 460 core

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

in vec3 fs_world_pos;
in vec2 fs_uv_pos;

out vec4 frag_color;

uniform MaterialShape material_shape;

void main(void) {
    vec3 color = material_shape.materials[0].color;
    frag_color = vec4(color, 1.0);
}
