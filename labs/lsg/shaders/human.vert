#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in uint in_region;

layout(location = 0) out vec3 surface_position_m;
layout(location = 1) out vec3 surface_normal;
layout(location = 2) flat out uint body_region;

layout(push_constant) uniform LsgPush {
    vec4 center_scale;
    vec4 render_params; // aspect, metres-per-source-unit, time, reserved
    uvec4 flags;        // character, detail, surface rotation, ui pass
} pc;

vec2 prerotate_clip(vec2 clip_position, uint rotation_code) {
    if (rotation_code == 1u) return vec2(-clip_position.y, clip_position.x);
    if (rotation_code == 2u) return -clip_position;
    if (rotation_code == 3u) return vec2(clip_position.y, -clip_position.x);
    return clip_position;
}

void emit_ui_vertex() {
    const vec2 corners[6] = vec2[](
        vec2(-1.0, -1.0), vec2( 1.0, -1.0), vec2( 1.0,  1.0),
        vec2(-1.0, -1.0), vec2( 1.0,  1.0), vec2(-1.0,  1.0));
    uint vertex = uint(gl_VertexIndex);
    uint button = min(vertex / 6u, 2u);
    vec2 corner = corners[vertex % 6u];
    vec2 center = button == 0u ? vec2(-0.55, -0.82)
                : button == 1u ? vec2( 0.00, -0.82)
                               : vec2( 0.55, -0.82);
    vec2 logical_clip = center + corner * vec2(0.22, 0.11);
    gl_Position = vec4(prerotate_clip(logical_clip, pc.flags.z), 0.01, 1.0);
    surface_position_m = vec3(corner * 0.5 + 0.5, 0.0);
    surface_normal = vec3(0.0, 0.0, 1.0);
    body_region = 100u + button;
}

void main() {
    if (pc.flags.w != 0u) {
        emit_ui_vertex();
        return;
    }

    vec3 local = in_position - pc.center_scale.xyz;
    uint character_index = pc.flags.x & 1u;

    float width_scale = 1.0;
    float depth_scale = 1.0;
    float height_scale = character_index == 0u ? 1.012 : 0.988;
    if (character_index == 0u) {
        if (in_region == 2u || in_region == 3u || in_region == 5u) width_scale = 1.055;
        if (in_region == 4u) width_scale = 0.990;
        if (in_region == 2u || in_region == 3u) depth_scale = 1.035;
    } else {
        if (in_region == 2u || in_region == 3u || in_region == 5u) width_scale = 0.965;
        if (in_region == 8u || in_region == 9u) width_scale = 1.035;
        if (in_region == 0u) width_scale = 1.012;
    }

    vec3 shaped = local;
    shaped.x *= width_scale;
    shaped.y *= height_scale;
    shaped.z *= depth_scale;

    float scale = pc.center_scale.w;
    float aspect = max(pc.render_params.x, 0.01);
    vec2 logical_clip = vec2(shaped.x * scale / aspect,
                             shaped.y * scale);
    gl_Position = vec4(prerotate_clip(logical_clip, pc.flags.z),
                       0.50 - shaped.z * scale * 0.12,
                       1.0);

    surface_position_m = shaped * pc.render_params.y;
    surface_normal = normalize(vec3(in_normal.x / width_scale,
                                    in_normal.y / height_scale,
                                    in_normal.z / depth_scale));
    body_region = in_region;
}
