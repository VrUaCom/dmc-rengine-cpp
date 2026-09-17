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
    vec4 render;       // aspect, vertical FOV radians, time, surface seed bits
    uvec4 flags;       // selected profile, detail enabled, surface rotation, UI pass
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
    gl_Position = vec4(logical_to_vulkan_clip(logical_clip, pc.flags.z), 0.01, 1.0);
    surface_position_m = vec3(corner * 0.5 + 0.5, 0.0);
    view_normal = vec3(0.0, 0.0, 1.0);
    body_region = 100u + button;
    view_position_m = vec3(0.0, 0.0, -1.0);
}

void main() {
    if (pc.flags.w != 0u) {
        emit_ui_vertex();
        return;
    }

    vec3 local = in_position - pc.center_units.xyz;

    float width_scale = pc.geometry1.z; // body-fat field applies softly everywhere.
    float depth_scale = pc.geometry1.z;
    if (in_region == 2u || in_region == 3u || in_region == 5u) {
        width_scale *= pc.geometry0.y * pc.geometry1.y;
        depth_scale *= pc.geometry0.w * pc.geometry1.y;
    } else if (in_region == 4u) {
        width_scale *= pc.geometry1.x;
    } else if (in_region == 8u || in_region == 9u) {
        width_scale *= pc.geometry0.z;
    } else if (in_region == 0u || in_region == 1u) {
        width_scale *= pc.geometry1.w;
        depth_scale *= pc.geometry1.w;
    }

    // Asset-free idle: deterministic breathing plus tiny posture/head motion.
    // The amplitudes are intentionally small so this remains a neutral R&D viewer.
    float t = pc.render.z;
    float breath_phase = t * 1.18 + 0.16 * sin(t * 0.31);
    float breath = sin(breath_phase);
    float chest_breath = (in_region == 2u || in_region == 3u || in_region == 5u) ? breath : 0.0;
    float abdomen_breath = in_region == 4u ? breath : 0.0;
    width_scale *= 1.0 + chest_breath * 0.0035 + abdomen_breath * 0.0018;
    depth_scale *= 1.0 + chest_breath * 0.0070 + abdomen_breath * 0.0040;

    if (in_region == 0u || in_region == 1u) {
        local = rotate_y(local, 0.0040 * sin(t * 0.37));
        local = rotate_x(local, 0.0025 * sin(t * 0.29 + 0.7));
    }

    vec3 shaped = local;
    shaped.x *= width_scale;
    shaped.y *= pc.geometry0.x;
    shaped.z *= depth_scale;
    vec3 object_m = shaped * pc.center_units.w;
    object_m.x += 0.0018 * sin(t * 0.43);
    object_m.y += 0.0007 * sin(t * 0.61 + 1.3);

    vec3 target_relative = object_m - vec3(0.0, pc.camera.w, 0.0);
    vec3 view = rotate_x(rotate_y(target_relative, -pc.camera.x), -pc.camera.y);
    view.z -= pc.camera.z;

    vec3 n = normalize(vec3(in_normal.x / max(width_scale, 0.001),
                            in_normal.y / max(pc.geometry0.x, 0.001),
                            in_normal.z / max(depth_scale, 0.001)));
    if (in_region == 0u || in_region == 1u) {
        n = normalize(rotate_x(rotate_y(n, 0.0040 * sin(t * 0.37)),
                               0.0025 * sin(t * 0.29 + 0.7)));
    }
    n = normalize(rotate_x(rotate_y(n, -pc.camera.x), -pc.camera.y));

    float aspect = max(pc.render.x, 0.01);
    float f = 1.0 / tan(max(pc.render.y, 0.10) * 0.5);
    const float near_z = 0.03;
    const float far_z = 30.0;
    vec4 clip;
    clip.x = view.x * f / aspect;
    clip.y = view.y * f;
    clip.z = (far_z / (near_z - far_z)) * view.z + (far_z * near_z / (near_z - far_z));
    clip.w = -view.z;
    clip.xy = logical_to_vulkan_clip(clip.xy, pc.flags.z);
    gl_Position = clip;

    surface_position_m = object_m;
    view_normal = n;
    body_region = in_region;
    view_position_m = view;
}
