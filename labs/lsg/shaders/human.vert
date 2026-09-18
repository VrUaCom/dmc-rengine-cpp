#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in uint in_region;

layout(location = 0) out vec3 surface_position_m;
layout(location = 1) out vec3 view_normal;
layout(location = 2) flat out uint body_region;
layout(location = 3) out vec3 view_position_m;

layout(push_constant) uniform LsgPush {
    vec4 center_units; // source-space center.xyz, metres-per-source-unit
    vec4 camera;       // yaw, pitch, distance metres, target Y metres
    vec4 geometry0;    // height, shoulders, pelvis, chest depth
    vec4 geometry1;    // waist, muscle, body fat, head
    vec4 skin0;        // melanin, haemoglobin, oiliness, hydration
    vec4 micro0;       // roughness bias, pore density, pore scale, pore depth
    vec4 render;       // logical aspect, vertical FOV radians, time, surface seed bits
    uvec4 flags;       // profile, detail, surface rotation, packed pass/mode
} pc;

// flags.w bit 0: UI/environment pass.
// flags.w bits 1..2: diagnostic render mode.
//   0 = genome + perspective
//   1 = raw mesh + perspective
//   2 = raw mesh + orthographic
//   3 = genome + MakeHuman joint debug geometry
uint diagnostic_mode() { return (pc.flags.w >> 1u) & 3u; }
bool ui_environment_pass() { return (pc.flags.w & 1u) != 0u; }

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

// One far-depth procedural environment triangle plus a compact extensible R&D list.
// Rows: Character 0, Character 1, Detail, Skeleton, Camera, Diagnostics, Reset View, Physiology.
// Holding a row exposes an on-screen tooltip encoded in flags.w bits 5..8.
void emit_ui_environment_vertex() {
    const vec2 full_triangle[3] = vec2[](
        vec2(-1.0, -1.0), vec2(3.0, -1.0), vec2(-1.0, 3.0));
    const vec2 quad[6] = vec2[](
        vec2(-1.0, -1.0), vec2( 1.0, -1.0), vec2( 1.0,  1.0),
        vec2(-1.0, -1.0), vec2( 1.0,  1.0), vec2(-1.0,  1.0));
    uint vertex = uint(gl_VertexIndex);

    if (vertex < 3u) {
        vec2 logical_clip = full_triangle[vertex];
        gl_Position = vec4(logical_to_vulkan_clip(logical_clip, pc.flags.z), 0.999, 1.0);
        surface_position_m = vec3(logical_clip, 0.0);
        view_normal = vec3(0.0, 1.0, 0.0);
        body_region = 200u;
        view_position_m = vec3(0.0, 0.0, -1.0);
        return;
    }

    if (vertex < 9u) {
        vec2 corner = quad[vertex - 3u];
        vec2 logical_clip = vec2(-0.81, 0.24) + corner * vec2(0.17, 0.70);
        gl_Position = vec4(logical_to_vulkan_clip(logical_clip, pc.flags.z), 0.012, 1.0);
        surface_position_m = vec3(corner * 0.5 + 0.5, 0.0);
        view_normal = vec3(0.0, 0.0, 1.0);
        body_region = 190u;
        view_position_m = vec3(0.0, 0.0, -1.0);
        return;
    }

    uint local_vertex = vertex - 9u;
    uint row = local_vertex / 6u;
    if (row < 8u) {
        vec2 corner = quad[local_vertex % 6u];
        float center_y = 0.80 - float(row) * 0.16;
        vec2 logical_clip = vec2(-0.81, center_y) + corner * vec2(0.13, 0.055);
        gl_Position = vec4(logical_to_vulkan_clip(logical_clip, pc.flags.z), 0.010, 1.0);
        surface_position_m = vec3(corner * 0.5 + 0.5, 0.0);
        view_normal = vec3(0.0, 0.0, 1.0);
        body_region = 100u + row;
        view_position_m = vec3(0.0, 0.0, -1.0);
        return;
    }

    uint tooltip = (pc.flags.w >> 5u) & 15u;
    if (vertex < 63u && tooltip < 8u) {
        vec2 corner = quad[vertex - 57u];
        vec2 logical_clip = vec2(0.08, -0.82) + corner * vec2(0.74, 0.075);
        gl_Position = vec4(logical_to_vulkan_clip(logical_clip, pc.flags.z), 0.008, 1.0);
        surface_position_m = vec3(corner * 0.5 + 0.5, 0.0);
        view_normal = vec3(0.0, 0.0, 1.0);
        body_region = 180u;
        view_position_m = vec3(0.0, 0.0, -1.0);
        return;
    }

    gl_Position = vec4(2.0, 2.0, 1.0, 1.0);
    surface_position_m = vec3(2.0);
    view_normal = vec3(0.0, 1.0, 0.0);
    body_region = 255u;
    view_position_m = vec3(0.0, 0.0, -1.0);
}

void main() {
    if (ui_environment_pass()) {
        emit_ui_environment_vertex();
        return;
    }

    uint mode = diagnostic_mode();
    bool joint_debug_mode = mode == 3u;

    // Region 255 is reserved by the debug merge tool for MakeHuman joint markers.
    // Hide those vertices before projection unless the explicit Skeleton toggle is active.
    if (in_region == 255u && !joint_debug_mode) {
        gl_Position = vec4(2.0, 2.0, 1.0, 1.0);
        surface_position_m = vec3(0.0);
        view_normal = vec3(0.0, 1.0, 0.0);
        body_region = 255u;
        view_position_m = vec3(0.0, 0.0, -1.0);
        return;
    }

    bool genome_mode = mode == 0u || joint_debug_mode;
    bool orthographic_mode = mode == 2u;

    vec3 raw_centered_m = (in_position - pc.center_units.xyz) * pc.center_units.w;
    vec3 object_m = raw_centered_m;
    vec3 n_object = normalize(in_normal);

    if (genome_mode) {
        float body_y01 = raw_centered_m.y / 1.75 + 0.5;
        AnatomyField weights = anatomy_field(body_y01);
        vec2 xz_scale = anatomy_xz_scales(weights);

        object_m.x *= xz_scale.x;
        object_m.y *= pc.geometry0.x;
        object_m.z *= xz_scale.y;

        float t = pc.render.z;
        float breath_phase = t * 1.18 + 0.16 * sin(t * 0.31);
        float breath = sin(breath_phase);
        float abdomen_weight = smooth_band(body_y01, 0.38, 0.45, 0.56, 0.64);
        float breath_width = 1.0 + breath * (weights.chest * 0.0035 + abdomen_weight * 0.0018);
        float breath_depth = 1.0 + breath * (weights.chest * 0.0070 + abdomen_weight * 0.0040);
        object_m.x *= breath_width;
        object_m.z *= breath_depth;

        object_m = apply_head_idle(object_m, weights.head, t);
        object_m.x += 0.0018 * sin(t * 0.43);
        object_m.y += 0.0007 * sin(t * 0.61 + 1.3);

        n_object = normalize(vec3(
            in_normal.x / max(xz_scale.x * breath_width, 0.001),
            in_normal.y / max(pc.geometry0.x, 0.001),
            in_normal.z / max(xz_scale.y * breath_depth, 0.001)));
        n_object = apply_head_idle_normal(n_object, weights.head, t);
    }

    vec3 target_relative = object_m - vec3(0.0, pc.camera.w, 0.0);
    vec3 view = rotate_x(rotate_y(target_relative, -pc.camera.x), -pc.camera.y);
    view.z -= pc.camera.z;
    vec3 n = normalize(rotate_x(rotate_y(n_object, -pc.camera.x), -pc.camera.y));

    float aspect = max(pc.render.x, 0.01);
    float fov = max(pc.render.y, 0.10);
    const float near_z = 0.03;
    const float far_z = 30.0;
    vec4 clip;

    if (orthographic_mode) {
        float half_height = max(0.08, pc.camera.z * tan(fov * 0.5));
        clip.x = view.x / max(half_height * aspect, 0.001);
        clip.y = view.y / max(half_height, 0.001);
        clip.z = ((-view.z) - near_z) / (far_z - near_z);
        clip.w = 1.0;
    } else {
        float f = 1.0 / tan(fov * 0.5);
        clip.x = view.x * f / aspect;
        clip.y = view.y * f;
        clip.z = (far_z / (near_z - far_z)) * view.z +
                 (far_z * near_z / (near_z - far_z));
        clip.w = -view.z;
    }

    clip.xy = logical_to_vulkan_clip(clip.xy, pc.flags.z);
    gl_Position = clip;

    surface_position_m = object_m;
    view_normal = n;
    body_region = in_region;
    view_position_m = view;
}
