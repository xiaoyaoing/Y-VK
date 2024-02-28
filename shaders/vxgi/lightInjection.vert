#version 460 core
#extension GL_GOOGLE_include_directive : require
#include "../perFrame.glsl"

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texCoord_0;

layout(location = 0) out vec3 position;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec2 texCoord;

void main() {
    vec4 pos = (per_primitive.model * vec4(in_position, 1.0f));
    gl_Position = (per_frame.view_proj * pos);

    position = pos.xyz / pos.w;
    texCoord = in_texCoord_0;
    normal =  mat3(per_primitive.model) * in_normal;
}