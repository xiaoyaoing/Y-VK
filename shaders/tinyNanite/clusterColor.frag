#version  320 es
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_debug_printf : enable
precision highp float;

layout(location = 0) in vec3 color;
layout(location = 0) out vec4 o_color;


void main(void)
{
    o_color = vec4(color, 1.0);
}