#ifndef RENGINE_LSG_FACE_FIELD_GLSL
#define RENGINE_LSG_FACE_FIELD_GLSL

#include "../include/rengine/lsg/face_field_contract.inc"

layout(set = 0, binding = 4, std140) uniform CharacterIdentity {
    vec4 face0;
    vec4 face1;
    vec4 face2;
    vec4 face3;
    vec4 face4;
    vec4 basis0; // head pivot, half width, half height, depth
    vec4 basis1; // eye half spacing, eye Y, field version, reserved
} identity;

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

float lsg_face_activation(float head_weight) {
    return lsg_face_smooth01(clamp(head_weight / RENGINE_FACE_ACTIVATION_FULL_WEIGHT, 0.0, 1.0));
}

vec3 lsg_face_canonical(vec3 raw_p) {
    return vec3(raw_p.x / identity.basis0.y,
                (raw_p.y - identity.basis0.x) / identity.basis0.z,
                raw_p.z / identity.basis0.w);
}

vec3 lsg_apply_face_field(vec3 p, vec3 raw_p, float head_weight) {
    float activation = lsg_face_activation(head_weight);
    if (activation <= 0.0) return p;

    vec3 q = lsg_face_canonical(raw_p);
    float front = lsg_face_smooth_range(q.z, RENGINE_FACE_FRONT_LOW, RENGINE_FACE_FRONT_HIGH);
    float center = 1.0 - lsg_face_smooth_range(abs(q.x), RENGINE_FACE_CENTER_LOW, RENGINE_FACE_CENTER_HIGH);
    float cheek = lsg_face_smooth_band(q.y, RENGINE_FACE_CHEEK_R0, RENGINE_FACE_CHEEK_R1, RENGINE_FACE_CHEEK_F0, RENGINE_FACE_CHEEK_F1) * front;
    float jaw = lsg_face_smooth_band(q.y, RENGINE_FACE_JAW_R0, RENGINE_FACE_JAW_R1, RENGINE_FACE_JAW_F0, RENGINE_FACE_JAW_F1) * front;
    float chin = lsg_face_smooth_band(q.y, RENGINE_FACE_CHIN_R0, RENGINE_FACE_CHIN_R1, RENGINE_FACE_CHIN_F0, RENGINE_FACE_CHIN_F1) * front;
    float eyes = lsg_face_smooth_band(q.y, RENGINE_FACE_EYES_R0, RENGINE_FACE_EYES_R1, RENGINE_FACE_EYES_F0, RENGINE_FACE_EYES_F1) * front;
    float nose = lsg_face_smooth_band(q.y, RENGINE_FACE_NOSE_R0, RENGINE_FACE_NOSE_R1, RENGINE_FACE_NOSE_F0, RENGINE_FACE_NOSE_F1) * front * center;
    float brow = lsg_face_smooth_band(q.y, RENGINE_FACE_BROW_R0, RENGINE_FACE_BROW_R1, RENGINE_FACE_BROW_F0, RENGINE_FACE_BROW_F1) * front;
    float forehead = lsg_face_smooth_band(q.y, RENGINE_FACE_FOREHEAD_R0, RENGINE_FACE_FOREHEAD_R1, RENGINE_FACE_FOREHEAD_F0, RENGINE_FACE_FOREHEAD_F1) * front;
    float mouth = lsg_face_smooth_band(q.y, RENGINE_FACE_MOUTH_R0, RENGINE_FACE_MOUTH_R1, RENGINE_FACE_MOUTH_F0, RENGINE_FACE_MOUTH_F1) * front;
    float upper_lip = lsg_face_smooth_band(q.y, RENGINE_FACE_UPPER_LIP_R0, RENGINE_FACE_UPPER_LIP_R1, RENGINE_FACE_UPPER_LIP_F0, RENGINE_FACE_UPPER_LIP_F1) * front * center;
    float lower_lip = lsg_face_smooth_band(q.y, RENGINE_FACE_LOWER_LIP_R0, RENGINE_FACE_LOWER_LIP_R1, RENGINE_FACE_LOWER_LIP_F0, RENGINE_FACE_LOWER_LIP_F1) * front * center;

    p.x *= 1.0 + RENGINE_FACE_SKULL_WIDTH_COEFF * identity.face0.x * activation;
    p.y = identity.basis0.x +
          (p.y - identity.basis0.x) * (1.0 + RENGINE_FACE_SKULL_HEIGHT_COEFF * identity.face0.y * activation);
    p.y += RENGINE_FACE_LENGTH_COEFF * identity.face0.z * (jaw + chin) * activation;
    p.y += RENGINE_FACE_FOREHEAD_HEIGHT_COEFF * identity.face0.w * forehead * activation;

    p.z += RENGINE_FACE_BROW_DEPTH_COEFF * identity.face1.x * brow * activation;
    p.x += sign(raw_p.x) * RENGINE_FACE_EYE_SPACING_COEFF * identity.face1.y * eyes * activation;
    p.x *= 1.0 + RENGINE_FACE_EYE_SIZE_SURFACE_COEFF * identity.face1.z * eyes * activation;
    p.y += sign(raw_p.x) * RENGINE_FACE_EYE_TILT_COEFF * identity.face1.w * eyes * activation;

    p.y -= RENGINE_FACE_NOSE_LENGTH_COEFF * identity.face2.x * nose * activation;
    p.x *= 1.0 + RENGINE_FACE_NOSE_WIDTH_COEFF * identity.face2.y * nose * activation;
    p.z += RENGINE_FACE_NOSE_PROJECTION_COEFF * identity.face2.z * nose * activation;
    p.x *= 1.0 + RENGINE_FACE_CHEEKBONE_WIDTH_COEFF * identity.face2.w * cheek * activation;

    p.z += RENGINE_FACE_CHEEK_FULLNESS_COEFF * identity.face3.x * cheek * activation;
    p.x *= 1.0 + RENGINE_FACE_JAW_WIDTH_COEFF * identity.face3.y * jaw * activation;
    p.x *= 1.0 + RENGINE_FACE_CHIN_WIDTH_COEFF * identity.face3.z * chin * activation;
    p.z += RENGINE_FACE_CHIN_PROJECTION_COEFF * identity.face3.w * chin * activation;

    p.x *= 1.0 + RENGINE_FACE_MOUTH_WIDTH_COEFF * identity.face4.x * mouth * activation;
    p.z += RENGINE_FACE_UPPER_LIP_COEFF * identity.face4.y * upper_lip * activation;
    p.z += RENGINE_FACE_LOWER_LIP_COEFF * identity.face4.z * lower_lip * activation;
    p.z += RENGINE_FACE_LIP_PROJECTION_COEFF * identity.face4.w * mouth * center * activation;
    return p;
}

vec3 lsg_apply_eye_socket_field(vec3 p, vec3 raw_p, float head_weight) {
    float activation = lsg_face_activation(head_weight);
    if (activation <= 0.0) return p;

    p.x *= 1.0 + RENGINE_FACE_SKULL_WIDTH_COEFF * identity.face0.x * activation;
    p.y = identity.basis0.x +
          (p.y - identity.basis0.x) * (1.0 + RENGINE_FACE_SKULL_HEIGHT_COEFF * identity.face0.y * activation);

    float side = raw_p.x < 0.0 ? -1.0 : 1.0;
    p.x += side * RENGINE_FACE_EYE_SPACING_COEFF * identity.face1.y * activation;
    float eye_center_x = side * identity.basis1.x;
    float eye_center_y = identity.basis1.y;
    p.x = eye_center_x + (p.x - eye_center_x) * (1.0 + RENGINE_FACE_EYE_SIZE_X_COEFF * identity.face1.z * activation);
    p.y = eye_center_y + (p.y - eye_center_y) * (1.0 + RENGINE_FACE_EYE_SIZE_Y_COEFF * identity.face1.z * activation);
    p.y += side * RENGINE_FACE_EYE_TILT_COEFF * identity.face1.w * activation;
    return p;
}

#endif
