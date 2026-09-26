#ifndef RENGINE_LSG_FACE_FIELD_GLSL
#define RENGINE_LSG_FACE_FIELD_GLSL

const float LSG_FACE_HEAD_PIVOT_Y = 0.690;
const float LSG_FACE_HALF_WIDTH_M = 0.100;
const float LSG_FACE_HALF_HEIGHT_M = 0.180;
const float LSG_FACE_DEPTH_M = 0.120;

float lsg_face_smooth01(float x) {
    x = clamp(x, 0.0, 1.0);
    return x * x * (3.0 - 2.0 * x);
}

float lsg_face_smooth_range(float value, float lo, float hi) {
    return lsg_face_smooth01((value - lo) / max(hi - lo, 1e-6));
}

float lsg_face_smooth_band(float value, float rise0, float rise1, float fall0, float fall1) {
    return lsg_face_smooth_range(value, rise0, rise1) *
           (1.0 - lsg_face_smooth_range(value, fall0, fall1));
}

vec3 lsg_face_canonical(vec3 raw_p) {
    return vec3(raw_p.x / LSG_FACE_HALF_WIDTH_M,
                (raw_p.y - LSG_FACE_HEAD_PIVOT_Y) / LSG_FACE_HALF_HEIGHT_M,
                raw_p.z / LSG_FACE_DEPTH_M);
}

vec3 lsg_apply_face_field(vec3 p, vec3 raw_p, float head_weight,
                          vec4 face0, vec4 face1, vec4 face2, vec4 face3, vec4 face4) {
    float activation = clamp(head_weight, 0.0, 1.0);
    if (activation <= 0.0) return p;

    vec3 q = lsg_face_canonical(raw_p);
    float front = lsg_face_smooth_range(q.z, 0.125, 0.750);
    float center = 1.0 - lsg_face_smooth_range(abs(q.x), 0.20, 0.75);
    float cheek = lsg_face_smooth_band(q.y, -0.306, -0.111, 0.250, 0.472) * front;
    float jaw = lsg_face_smooth_band(q.y, -0.750, -0.556, -0.139, 0.111) * front;
    float chin = lsg_face_smooth_band(q.y, -0.861, -0.722, -0.444, -0.250) * front;
    float eyes = lsg_face_smooth_band(q.y, 0.083, 0.222, 0.500, 0.667) * front;
    float nose = lsg_face_smooth_band(q.y, -0.361, -0.167, 0.472, 0.639) * front * center;
    float brow = lsg_face_smooth_band(q.y, 0.333, 0.444, 0.611, 0.750) * front;
    float forehead = lsg_face_smooth_band(q.y, 0.472, 0.611, 0.889, 1.028) * front;
    float mouth = lsg_face_smooth_band(q.y, -0.472, -0.333, 0.000, 0.139) * front;
    float upper_lip = lsg_face_smooth_band(q.y, -0.267, -0.211, -0.122, -0.056) * front * center;
    float lower_lip = lsg_face_smooth_band(q.y, -0.378, -0.300, -0.206, -0.139) * front * center;

    p.x *= 1.0 + 0.060 * face0.x * activation;
    p.y = LSG_FACE_HEAD_PIVOT_Y +
          (p.y - LSG_FACE_HEAD_PIVOT_Y) * (1.0 + 0.050 * face0.y * activation);
    p.y += 0.014 * face0.z * (jaw + chin) * activation;
    p.y += 0.010 * face0.w * forehead * activation;

    p.z += 0.010 * face1.x * brow * activation;
    p.x += sign(raw_p.x) * 0.008 * face1.y * eyes * activation;
    p.x *= 1.0 + 0.030 * face1.z * eyes * activation;
    p.y += sign(raw_p.x) * 0.006 * face1.w * eyes * activation;

    p.y -= 0.009 * face2.x * nose * activation;
    p.x *= 1.0 + 0.120 * face2.y * nose * activation;
    p.z += 0.020 * face2.z * nose * activation;
    p.x *= 1.0 + 0.080 * face2.w * cheek * activation;

    p.z += 0.012 * face3.x * cheek * activation;
    p.x *= 1.0 + 0.085 * face3.y * jaw * activation;
    p.x *= 1.0 + 0.100 * face3.z * chin * activation;
    p.z += 0.014 * face3.w * chin * activation;

    p.x *= 1.0 + 0.080 * face4.x * mouth * activation;
    p.z += 0.009 * face4.y * upper_lip * activation;
    p.z += 0.010 * face4.z * lower_lip * activation;
    p.z += 0.008 * face4.w * mouth * center * activation;
    return p;
}

vec3 lsg_apply_eye_socket_field(vec3 p, vec3 raw_p, float head_weight,
                                vec4 face0, vec4 face1) {
    float activation = clamp(head_weight, 0.0, 1.0);
    if (activation <= 0.0) return p;

    p.x *= 1.0 + 0.060 * face0.x * activation;
    p.y = LSG_FACE_HEAD_PIVOT_Y +
          (p.y - LSG_FACE_HEAD_PIVOT_Y) * (1.0 + 0.050 * face0.y * activation);

    float side = raw_p.x < 0.0 ? -1.0 : 1.0;
    p.x += side * 0.008 * face1.y * activation;
    float eye_center_x = side * 0.032;
    const float eye_center_y = 0.752;
    p.x = eye_center_x + (p.x - eye_center_x) * (1.0 + 0.070 * face1.z * activation);
    p.y = eye_center_y + (p.y - eye_center_y) * (1.0 + 0.055 * face1.z * activation);
    p.y += side * 0.006 * face1.w * activation;
    return p;
}

#endif
