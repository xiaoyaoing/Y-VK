#version 320 es

#extension GL_GOOGLE_include_directive : enable
#include "vxgi_common.h"
#include "../perFrame.glsl"

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in uint primitive_id;

layout (location = 0) out VS_OUT {
    vec3 normal;
} vs_out;


layout(std430, set = 0, binding = 2) readonly buffer _GlobalPrimitiveUniform {
    PerPrimitive primitive_infos[];
};

void main(void)
{
    mat4  matrix = primitive_infos[primitive_id].model;
    vec4 pos = (matrix * vec4(position, 1.0f));
    gl_Position = pos;
    vs_out.normal = mat3(primitive_infos[primitive_id].modelIT) * normal;
}
