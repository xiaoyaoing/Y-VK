#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 outColor;
void main() {
    outColor = vec4(uv.x, uv.y, 0.0, 1.0);
}