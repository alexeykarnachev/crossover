#version 460 core

const vec2 VERTICES[4] = {
    vec2(-1.0, -1.0),
    vec2(1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, 1.0)
};

out vec2 fs_uv_pos;

void main() {
    vec2 pos = VERTICES[gl_VertexID];
    fs_uv_pos = 0.5 * (pos + 1.0);
    gl_Position = vec4(pos, 0.0, 1.0);
}

