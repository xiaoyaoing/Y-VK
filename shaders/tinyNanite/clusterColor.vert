#version  320 es
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_debug_printf : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout (location = 0) out vec3 o_color;

#include "../perFrame.glsl"
void main(void)
{
    gl_Position = per_frame.view_proj * vec4(position, 1.0);
   // gl_Position =  vec4(position, 1.0);
    o_color = color;
}