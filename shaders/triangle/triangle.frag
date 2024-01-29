#version 320 es

precision highp float;

layout(location = 0) in vec3 in_color;

layout(location = 0) out vec4 o_color;
void main()
{
    o_color = vec4(in_color, 1.0);
}