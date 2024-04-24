#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord_0;

#include "perframe.glsl"

layout (location = 0) out vec3 outUVW;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    outUVW = position;
    //gl_Position = per_frame.view_proj * vec4(position.xyz, 1.0);
    gl_Position = per_frame.proj * vec4(mat3(per_frame.view) * position.xyz, 1.0);
}
