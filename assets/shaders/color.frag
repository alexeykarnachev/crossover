#version 460 core

in vec2 fs_uv_pos;

uniform sampler2D world_pos_tex;
uniform sampler2D normals_tex;
uniform sampler2D diffuse_tex;

out vec4 frag_color;

vec3 light_dir = vec3(1.0, 1.0, -1.0);
vec3 light_color = vec3(1.0, 1.0, 1.0);

void main(void) {
    vec2 uv = fs_uv_pos;
    vec3 world_pos = texture(world_pos_tex, uv).xyz;
    vec3 normal = texture(normals_tex, uv).xyz;
    vec3 diffuse_color = texture(diffuse_tex, uv).xyz;
    light_dir = normalize(light_dir);
    float diffuse = max(dot(normal, -light_dir), 0.0);
    vec3 color;
    if (length(normal) > 0.0) {
        normal = normalize(normal);
        color = diffuse_color * diffuse * light_color; 
    } else {
        color = diffuse_color;
    }

    frag_color = vec4(color, 1.0);
    frag_color = vec4(diffuse_color, 1.0);
}
