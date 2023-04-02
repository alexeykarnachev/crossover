#version 460 core

in vec3 fs_world_pos;
in vec2 fs_uv_pos;
in vec3 fs_color;

out vec4 frag_color;

void main(void) {
    frag_color = vec4(fs_color, 1.0);
}
