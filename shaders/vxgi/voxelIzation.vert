#version 320 es

#extension GL_GOOGLE_include_directive : enable
#include "vxgi_common.h"
#include "../perFrame.glsl"

layout(location = 0) in vec3 position;


//layout (location = 0) out vec3 o_normal;

void main(void)
{
    vec4 pos = (per_primitive.model * vec4(position, 1.0f));
    gl_Position = (per_frame.view_proj * pos);
    //gl_Position.z = 1.f / gl_Position.z;
}
