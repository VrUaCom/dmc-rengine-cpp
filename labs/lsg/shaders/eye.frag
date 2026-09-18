#version 450

layout(location = 0) in vec3 view_normal;
layout(location = 1) in vec2 eye_uv;
layout(location = 2) flat in uint component_id;

layout(location = 0) out vec4 out_colour;

vec3 component_colour(uint id) {
    if (id == 0u) return vec3(0.95, 0.18, 0.16);
    if (id == 1u) return vec3(0.12, 0.78, 0.30);
    if (id == 2u) return vec3(0.18, 0.38, 0.96);
    if (id == 3u) return vec3(0.96, 0.72, 0.10);
    return vec3(0.85, 0.20, 0.85);
}

void main() {
    vec3 n = normalize(view_normal);
    vec3 light_dir = normalize(vec3(0.35, 0.70, 0.55));
    float ndotl = max(dot(n, light_dir), 0.0);
    float rim = pow(1.0 - abs(n.z), 2.0);
    vec3 base = component_colour(component_id);
    vec3 colour = base * (0.42 + 0.58 * ndotl) + vec3(0.12) * rim;
    out_colour = vec4(colour, 0.66);
}
