#version 450
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_debug_printf : enable

#include "../perFrame.glsl"

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord_0;
layout(location = 3) in uint primitive_id;

//layout (location = 1) out vec2 o_uv;
//layout (location = 2) out vec3 o_normal;
//layout (location = 3) out  flat uint o_primitive_id;

layout( location = 0 ) out VS_OUT {
    vec2 texCoord;
    vec3 normal;
    uint i_primID;
} vs_out;



layout(std430, set = 0, binding = 2) readonly buffer _GlobalPrimitiveUniform {
    PerPrimitive primitive_infos[];
};

void main(void)
{
    mat4  matrix = primitive_infos[primitive_id].model;
    
    gl_Position =  matrix * vec4(position, 1.0f);
    vs_out.texCoord = texcoord_0;
    vs_out.normal = normalize(mat3(primitive_infos[primitive_id].modelIT) * normal);
    vs_out.i_primID = primitive_id;
}