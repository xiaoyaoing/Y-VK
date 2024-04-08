#version 320 es

#extension GL_GOOGLE_include_directive : enable
#include "vxgi_common.h"
#include "../perFrame.glsl"

layout(location = 0) in vec3 position;
layout(location = 1) in uint primitive_id;

layout (location = 0) out vec3 o_position;


layout(std430, set = 0, binding = 2) readonly buffer _GlobalPrimitiveUniform {
    PerPrimitive primitive_infos[];
};

void main(void)
{
    mat4  matrix = primitive_infos[primitive_id].model;
    vec4 pos = (matrix * vec4(position, 1.0f));
    gl_Position = (per_frame.view_proj * pos);
    o_position = pos.xyz;
    //gl_Position.z = 1.f / gl_Position.z;
}
