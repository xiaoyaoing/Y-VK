#version  320 es
#extension GL_GOOGLE_include_directive : enable
#include "../perframe.glsl"


layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

layout (location = 0) out vec3 o_color;
void main()
{
    //gl_Position = per_frame.view_proj * per_primitive.model * vec4(position, 1.0);
    gl_Position = per_frame.view_proj * per_primitive.model * vec4(position, 1.0);
    o_color = color;
}