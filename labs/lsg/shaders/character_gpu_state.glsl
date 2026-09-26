#ifndef RENGINE_LSG_CHARACTER_GPU_STATE_GLSL
#define RENGINE_LSG_CHARACTER_GPU_STATE_GLSL

layout(set = 0, binding = 3, std140) uniform CharacterGpuState {
    vec4 face0;
    vec4 face1;
    vec4 face2;
    vec4 face3;
    vec4 face4;
    vec4 basis0;
    vec4 basis1;
    vec4 pigments;
    vec4 surface;
    vec4 pores;
    vec4 features;
    vec4 physiology;
} character_state;

#endif
