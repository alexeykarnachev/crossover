#version 460 core

uniform int light_idx;

layout(location=0) out float light_mask_tex;

void main(void) {
    light_mask_tex = float(1 << light_idx);
}

