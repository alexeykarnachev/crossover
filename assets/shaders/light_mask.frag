#version 460 core

uniform int light_idx;

layout(location=0) out vec3 light_mask_tex;

void main(void) {
    light_mask_tex = vec3(float(1 << light_idx), 0.0, 0.0);
}

