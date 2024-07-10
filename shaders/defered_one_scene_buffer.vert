#version  320 es
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_debug_printf : enable

#include "perFrame.glsl"
//#include "common/sampling.glsl"
precision highp float;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord_0;
layout(location = 3) in uint primitive_id;

layout (location = 0) out vec2 o_uv;
layout (location = 1) out vec3 o_normal;
layout (location = 2) out  flat uint o_primitive_id;
layout (location = 3) out  vec3 o_position;


layout(std430, set = 0, binding = 2) readonly buffer _GlobalPrimitiveUniform {
    PerPrimitive primitive_infos[];
};

layout(std430, set = 0, binding = 5) readonly buffer _BLUE_NOISE {
    vec4 blue_noise[];
};



layout(push_constant) uniform PushConstant {
    uint  frame_index;
    uint jitter;
    ivec2 screen_size;
    uint use_stochastic;
    uint padding[3];
} pc;



vec2 uv_jitter(){
//    return vec2(0,0);

    if(pc.jitter == 0u) return vec2(0.f,0.f);
    int index = int(pc.frame_index);
    index = index % (128 * 128);
    ivec2 uv = ivec2(index % 128, index / 128);
    vec2 uv_float = vec2(uv) + vec2(0.5f);
   // debugPrintfEXT("uv_float: %f %f\n", uv_float.x, uv_float.y);
 return blue_noise[index].xy / 255.f;
   
    }

void main(void)
{
    mat4  matrix = primitive_infos[primitive_id].model;
    mat4  matrixIT = primitive_infos[primitive_id].modelIT;
    vec4 pos = matrix  * vec4(position, 1.0f);
    
//    o_uv = fract(fract((texcoord_0)) * 100.f);
    o_uv = texcoord_0;// * 100.f;
//    if(o_uv!=vec2(0) && o_uv!=vec2(100,0) && o_uv!=vec2(0,100) && o_uv!=vec2(100,100))
//    {
//        debugPrintfEXT("uv: %f %f\n", o_uv.x, o_uv.y);
//    }
//    o_uv = o_uv -  floor(o_uv);
//    debugPrintfEXT("pos: %f %f %f\n", pos.x, pos.y, pos.z);

    o_normal = normalize((matrix * vec4(normal, 0.0f)).xyz);

    o_primitive_id = primitive_id;

    o_position =    pos.xyz;

    gl_Position = per_frame.view_proj * pos;
    gl_Position.xy += (uv_jitter() - vec2(0.5f)) / vec2(pc.screen_size);
    vec2 jitter = uv_jitter();
   // debugPrintfEXT("jitter: %f %f\n", jitter.x, jitter.y);
}