#version 450

layout(binding = 1) uniform sampler2D texSampler;


layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 uv;


layout(location = 0) out vec4 outColor;
void main() {
    outColor = texture(texSampler, uv);
    //gl_FragDepth = 0.5;
  //  gl_FragCoord.z = 0.5;
   // outColor = vec4(0,  gl_FragDepth, 0, 1);
}