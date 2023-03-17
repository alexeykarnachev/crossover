#version 460 core

struct ColorMaterial {
    vec3 color;
};

// struct BrickMaterial {
// };
// 
// struct StoneMaterial {
// };

in vec2 fs_uv_pos;

uniform int material_type;
uniform ColorMaterial color_material;
// uniform BrickMaterial brick_material;
// uniform StoneMaterial stone_material;

out vec4 frag_color;


vec3 get_brick_color(void) {
    return vec3(fs_uv_pos, 0.0);
}

void main(void) {
    vec3 color = vec3(0.0);
    switch (material_type) {
        case 0:
            color = color_material.color;
            break;
        case 1:
            color = get_brick_color();
            break;
        default:
            break;
    }
    frag_color = vec4(color, 1.0);
}
