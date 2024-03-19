#version  320 es
#extension GL_GOOGLE_include_directive : enable


#include "perFrame.glsl"

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord_0;


layout (location = 0) out vec2 o_uv;
layout (location = 1) out vec3 o_normal;

void main(void)
{
    vec4 pos = per_primitive.model * vec4(position, 1.0f);

    o_uv = texcoord_0;

    o_normal = mat3(per_primitive.model) * normal;

    gl_Position = per_frame.view_proj * pos;
}

