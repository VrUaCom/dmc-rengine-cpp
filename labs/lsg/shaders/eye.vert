#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in uint in_component;

layout(location = 0) out vec3 view_normal;
layout(location = 1) out vec2 eye_uv;
layout(location = 2) flat out uint component_id;
layout(location = 3) out vec3 view_position;

layout(push_constant) uniform EyePush {
    vec4 center_units; // body source-space center.xyz, metres-per-source-unit
    vec4 camera;       // yaw, pitch, distance metres, target Y metres
    vec4 geometry0;    // height, shoulders, pelvis, chest depth
    vec4 geometry1;    // waist, muscle, body fat, head
    vec4 render;       // logical aspect, vertical FOV radians, time, reserved
    vec4 eye0;         // primary iris rgb, pupil bias
    vec4 eye1;         // secondary iris rgb, sclera tint
    uvec4 flags;       // surface rotation, profile, vascularity byte, eye seed low
} pc;

vec2 prerotate_clip(vec2 clip_position, uint rotation_code) {
    if (rotation_code == 1u) return vec2(-clip_position.y, clip_position.x);
    if (rotation_code == 2u) return -clip_position;
    if (rotation_code == 3u) return vec2(clip_position.y, -clip_position.x);
    return clip_position;
}

vec2 logical_to_vulkan_clip(vec2 logical_clip, uint rotation_code) {
    return prerotate_clip(vec2(logical_clip.x, -logical_clip.y), rotation_code);
}

vec3 rotate_y(vec3 v, float a) {
    float c = cos(a), s = sin(a);
    return vec3(c * v.x + s * v.z, v.y, -s * v.x + c * v.z);
}

vec3 rotate_x(vec3 v, float a) {
    float c = cos(a), s = sin(a);
    return vec3(v.x, c * v.y - s * v.z, s * v.y + c * v.z);
}

float smooth01(float x) {
    x = clamp(x, 0.0, 1.0);
    return x * x * (3.0 - 2.0 * x);
}

float smooth_range(float value, float lo, float hi) {
    return smooth01((value - lo) / max(hi - lo, 1e-6));
}

float smooth_band(float value, float rise0, float rise1, float fall0, float fall1) {
    return smooth_range(value, rise0, rise1) * (1.0 - smooth_range(value, fall0, fall1));
}

struct AnatomyField {
    float shoulder;
    float chest;
    float waist;
    float pelvis;
    float head;
};

AnatomyField anatomy_field(float body_y01) {
    float y = clamp(body_y01, 0.0, 1.0);
    AnatomyField w;
    w.shoulder = smooth_band(y, 0.55, 0.64, 0.78, 0.87);
    w.chest = smooth_band(y, 0.50, 0.58, 0.70, 0.79);
    w.waist = smooth_band(y, 0.39, 0.46, 0.55, 0.63);
    w.pelvis = smooth_band(y, 0.29, 0.36, 0.47, 0.55);
    w.head = smooth_range(y, 0.78, 0.89);
    return w;
}

vec2 anatomy_xz_scales(AnatomyField w) {
    float fat_delta = pc.geometry1.z - 1.0;
    float muscle_delta = pc.geometry1.y - 1.0;

    float width_scale = 1.0 + 0.55 * fat_delta;
    float depth_scale = 1.0 + 0.70 * fat_delta;

    width_scale += w.shoulder * ((pc.geometry0.y - 1.0) + 0.55 * muscle_delta);
    width_scale += w.waist * (pc.geometry1.x - 1.0);
    width_scale += w.pelvis * (pc.geometry0.z - 1.0);
    width_scale += w.head * (pc.geometry1.w - 1.0);

    depth_scale += w.chest * ((pc.geometry0.w - 1.0) + 0.35 * muscle_delta);
    depth_scale += w.waist * 0.50 * (pc.geometry1.x - 1.0);
    depth_scale += w.pelvis * 0.50 * (pc.geometry0.z - 1.0);
    depth_scale += w.head * (pc.geometry1.w - 1.0);

    return clamp(vec2(width_scale, depth_scale), vec2(0.75), vec2(1.25));
}

vec3 apply_head_idle(vec3 p, float head_weight, float t) {
    const float neck_pivot_y = 0.525;
    vec3 relative = p - vec3(0.0, neck_pivot_y, 0.0);
    vec3 rotated = rotate_y(relative, 0.0040 * sin(t * 0.37));
    rotated = rotate_x(rotated, 0.0025 * sin(t * 0.29 + 0.7));
    rotated += vec3(0.0, neck_pivot_y, 0.0);
    return mix(p, rotated, head_weight);
}

vec3 apply_head_idle_normal(vec3 n, float head_weight, float t) {
    vec3 rotated = rotate_y(n, 0.0040 * sin(t * 0.37));
    rotated = rotate_x(rotated, 0.0025 * sin(t * 0.29 + 0.7));
    return normalize(mix(n, rotated, head_weight));
}

void main() {
    vec3 raw_centered_m = (in_position - pc.center_units.xyz) * pc.center_units.w;
    vec3 object_m = raw_centered_m;
    vec3 n_object = normalize(in_normal);

    float body_y01 = raw_centered_m.y / 1.75 + 0.5;
    AnatomyField weights = anatomy_field(body_y01);
    vec2 xz_scale = anatomy_xz_scales(weights);

    object_m.x *= xz_scale.x;
    object_m.y *= pc.geometry0.x;
    object_m.z *= xz_scale.y;

    float t = pc.render.z;
    object_m = apply_head_idle(object_m, weights.head, t);
    object_m.x += 0.0018 * sin(t * 0.43);
    object_m.y += 0.0007 * sin(t * 0.61 + 1.3);

    n_object = normalize(vec3(
        in_normal.x / max(xz_scale.x, 0.001),
        in_normal.y / max(pc.geometry0.x, 0.001),
        in_normal.z / max(xz_scale.y, 0.001)));
    n_object = apply_head_idle_normal(n_object, weights.head, t);

    vec3 target_relative = object_m - vec3(0.0, pc.camera.w, 0.0);
    vec3 view = rotate_x(rotate_y(target_relative, -pc.camera.x), -pc.camera.y);
    view.z -= pc.camera.z;
    vec3 n = normalize(rotate_x(rotate_y(n_object, -pc.camera.x), -pc.camera.y));

    float aspect = max(pc.render.x, 0.01);
    float fov = max(pc.render.y, 0.10);
    const float near_z = 0.03;
    const float far_z = 30.0;
    float f = 1.0 / tan(fov * 0.5);

    vec4 clip;
    clip.x = view.x * f / aspect;
    clip.y = view.y * f;
    clip.z = (far_z / (near_z - far_z)) * view.z +
             (far_z * near_z / (near_z - far_z));
    clip.w = -view.z;
    clip.xy = logical_to_vulkan_clip(clip.xy, pc.flags.x);

    gl_Position = clip;
    view_normal = n;
    eye_uv = in_uv;
    component_id = in_component;
    view_position = view;
}
