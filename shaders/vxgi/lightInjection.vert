#version 450
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_debug_printf : enable

#include "../perFrame.glsl"

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord_0;
layout(location = 3) in uint primitive_id;

layout(location = 0) out vec3 o_position;
layout(location = 4) out vec3 o_position_1;
layout (location = 1) out vec2 o_uv;
layout (location = 2) out vec3 o_normal;
layout (location = 3) out  flat uint o_primitive_id;

layout(push_constant) uniform PushConstant
{
    uint frame_index;
};


layout(std430, set = 0, binding = 2) readonly buffer _GlobalPrimitiveUniform {
    PerPrimitive primitive_infos[];
};

void main(void)
{
    mat4  matrix = primitive_infos[primitive_id].model;
    vec4 pos =  matrix * vec4(position, 1.0f);
    
    o_position_1 = position.xyz;
    o_position = pos.xyz;

    o_uv = texcoord_0;

    o_normal = mat3(primitive_infos[primitive_id].modelIT) * normal;

    o_primitive_id = primitive_id;

    gl_Position = per_frame.view_proj * pos;

    vec3 p = vec3(5.705555 ,9.622492, -2.028390);
    vec3 pl = p-0.5f;
    vec3 pu = p+0.5f;
    if(all(greaterThan( o_position, pl)) && all(lessThan( o_position, pu)))
    debugPrintfEXT(" o_position %f %f %f frame_index %d\n", o_position.x, o_position.y, o_position.z, frame_index);
    //gl_Position.z = 1.f / gl_Position.z;
}