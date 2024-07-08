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
layout (location = 3) out  vec3 o_position;


layout(std430, set = 0, binding = 2) readonly buffer _GlobalPrimitiveUniform {
    PerPrimitive primitive_infos[];
};

layout(push_constant) uniform PushConstant {
    uint  frame_index;
        uint pad;
    ivec2 screen_size;
} pc;

vec2 uv_jitter(){
    return vec2(0,0);

    int index = int(pc.frame_index);
    index = index % 16;
   if(index == 0) return vec2(0.500000, 0.333333);
    else if(index == 1) return vec2(0.250000, 0.666667);
    else if(index == 2) return vec2(0.750000, 0.111111);
    else if(index == 3) return vec2(0.125000, 0.444444);
    else if(index == 4) return vec2(0.625000, 0.777778);
    else if(index == 5) return vec2(0.375000, 0.222222);
    else if(index == 6) return vec2(0.875000, 0.555556);
    else if(index == 7) return vec2(0.062500, 0.888889);
    else if(index == 8) return vec2(0.562500, 0.037037);
    else if(index == 9) return vec2(0.312500, 0.370370);
    else if(index == 10) return vec2(0.812500, 0.703704);
    else if(index == 11) return vec2(0.187500, 0.148148);
    else if(index == 12) return vec2(0.687500, 0.481481);
    else if(index == 13) return vec2(0.437500, 0.814815);
    else if(index == 14) return vec2(0.937500, 0.259259);
    else if(index == 15) return vec2(0.031250, 0.592593);
    return vec2(0,0);
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
}