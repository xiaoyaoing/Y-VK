#version 100

#include "../perFrame.glsl"

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texCoord;
layout (location = 3) in vec4 aTangent;

layout( location = 0 ) out VS_OUT {
	vec2 texCoord;
	vec3 normal;
} vs_out;

void main() {
    vec4 pos = (per_primitive.model * vec4(in_position, 1.0f));
    gl_Position = (per_frame.view_proj * pos);
    vs_out.texCoord = in_texCoord;
    vs_out.normal =  mat3(per_primitive.model) * in_normal;
}