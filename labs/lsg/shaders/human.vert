#version 450

layout(location = 0) out vec2 surface_position_m;
layout(location = 1) out vec3 bary_colour;

const vec2 clip_positions[3] = vec2[](
    vec2(-0.72,  0.66),
    vec2(-0.72, -0.66),
    vec2( 0.72, -0.66));

// A stable rest/object-space patch measured in metres. The fragment stage uses
// this coordinate for deterministic procedural detail; never gl_FragCoord.
const vec2 rest_positions_m[3] = vec2[](
    vec2(0.000, 0.000),
    vec2(0.000, 0.180),
    vec2(0.180, 0.180));

const vec3 colours[3] = vec3[](
    vec3(1.00, 0.25, 0.20),
    vec3(0.18, 1.00, 0.30),
    vec3(0.22, 0.36, 1.00));

void main() {
    gl_Position = vec4(clip_positions[gl_VertexIndex], 0.0, 1.0);
    surface_position_m = rest_positions_m[gl_VertexIndex];
    bary_colour = colours[gl_VertexIndex];
}
