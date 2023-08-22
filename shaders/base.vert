#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 in_uv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 out_uv;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;


void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
   // gl_Position = vec4(inPosition,1.0);
  // gl_Position.w = 1.0;
  //  gl_Position.z = 1;
    fragColor = inColor;
   // out_uv = vec2(gl_Position.z,gl_Position;
    out_uv = in_uv;
    //    out_uv = vec2(gl_Position.z/10, 0);
}