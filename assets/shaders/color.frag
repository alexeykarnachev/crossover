#version 460 core

in vec2 fs_uv_pos;

uniform sampler2D world_pos_tex;
uniform sampler2D diffuse_tex;

out vec4 frag_color;

void main(void) {
    vec2 uv = fs_uv_pos;
    vec4 diffuse_color = texture(diffuse_tex, uv);
    vec3 world_pos = texture(world_pos_tex, uv).xyz;

    frag_color = diffuse_color; 
}
