#version 460 core

#define MAX_N_LIGHTS 64

struct Light {
    vec3 color;
    vec3 attenuation;
    vec3 vec;
    float power;
    int is_dir;
};

in vec2 fs_uv_pos;

uniform sampler2D world_pos_tex;
uniform sampler2D normals_tex;
uniform sampler2D diffuse_tex;

uniform int enable_lights;
uniform int n_lights;
uniform Light lights[MAX_N_LIGHTS];

out vec4 frag_color;

void main(void) {
    vec2 uv = fs_uv_pos;
    vec3 world_pos = texture(world_pos_tex, uv).xyz;
    vec3 normal = texture(normals_tex, uv).xyz;
    vec3 diffuse_color = texture(diffuse_tex, uv).xyz;

    vec3 color = vec3(0.0);
    if (enable_lights == 1 && length(normal) > 0.000001) {
        normal = normalize(normal);

        for (int i = 0; i < min(MAX_N_LIGHTS, n_lights); ++i) {
            Light light = lights[i];
            
            vec3 light_dir;
            float attenuation;
            if (light.is_dir == 1) {
                light_dir = normalize(light.vec);
                attenuation = 1.0;
            } else {
                vec3 light_to_pos = world_pos - light.vec;
                float light_dist = length(light_to_pos);
                light_dir = normalize(light_to_pos);
                attenuation = 1.0 / dot(light.attenuation, vec3(1.0, light_dist, light_dist * light_dist));
            }

            float diffuse = max(dot(normal, -light_dir), 0.0);
            color += diffuse_color * diffuse * light.color * light.power * attenuation;

        }
    } else {
        color = diffuse_color;
    }

    frag_color = vec4(color, 1.0);
}
