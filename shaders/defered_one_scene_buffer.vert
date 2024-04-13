#version  320 es
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_debug_printf : enable

#include "perFrame.glsl"

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord_0;
layout(location = 3) in uint primitive_id;

layout (location = 0) out vec2 o_uv;
layout (location = 1) out vec3 o_normal;
layout (location = 2) out  flat uint o_primitive_id;


layout(std430, set = 0, binding = 2) readonly buffer _GlobalPrimitiveUniform {
    PerPrimitive primitive_infos[];
};

void main(void)
{
    mat4  matrix = primitive_infos[primitive_id].model;
    mat4  matrixIT = primitive_infos[primitive_id].modelIT;
    vec4 pos = matrix  * vec4(position, 1.0f);

    vec3 p = vec3(5.705555 ,9.622492, -2.028390);
    vec3 pl = p-0.5f;
    vec3 pu = p+0.5f;
    if(all(greaterThan( pos.xyz, pl)) && all(lessThan( pos.xyz, pu)))
    debugPrintfEXT(" pos_defered %f %f %f frame_index \n", pos.x, pos.y, pos.z);

    o_uv = texcoord_0;

    o_normal = (matrixIT * vec4(normal, 1.0f)).xyz;

    o_primitive_id = primitive_id;

    gl_Position = per_frame.view_proj * pos;
   // gl_Position.y *= -1.f;
    //gl_Position.z = 1.f / gl_Position.z;
}