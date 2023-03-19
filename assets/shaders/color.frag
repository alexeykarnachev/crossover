#version 460 core

in vec2 fs_uv_pos;

uniform sampler2D world_pos_tex;
uniform sampler2D diffuse_tex;

out vec4 frag_color;

void main(void) {
    vec4 diffuse_color = texture(diffuse_tex, fs_uv_pos);
    vec2 world_pos = texture(world_pos_tex, fs_uv_pos).rg;
    frag_color = diffuse_color; 
}
