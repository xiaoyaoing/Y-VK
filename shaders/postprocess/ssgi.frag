#version 460 core

#extension GL_GOOGLE_include_directive : enable 
#extension GL_EXT_debug_printf : enable
#extension GL_EXT_nonuniform_qualifier : enable


#include "../perFrame.glsl"
#include "../common/sampling.glsl"

precision highp float;

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_color;


layout(input_attachment_index = 0, binding = 0, set=2) uniform subpassInput gbuffer_diffuse_roughness;
layout(input_attachment_index = 1, binding = 1, set=2) uniform subpassInput gbuffer_normal_metalic;
layout(input_attachment_index = 2, binding = 2, set=2) uniform subpassInput gbuffer_emission;
layout(binding = 3, set=1) uniform sampler2D  gbuffer_depth;
layout(binding = 4, set=1) uniform sampler2D frame_color;

uint jenkinsHash(uint a)
{
    a = (a + 0x7ed55d16) + (a << 12);
    a = (a ^ 0xc761c23cu) ^ (a >> 19);
    a = (a + 0x165667b1) + (a << 5);
    a = (a + 0xd3a2646c) ^ (a << 9);
    a = (a + 0xfd7046c5) + (a << 3);
    a = (a ^ 0xb55a4f09u) ^ (a >> 16);
    return a;
}

vec3 pseudocolor(uint value)
{
    if (value == 0)
    return vec3(1, 0, 0);
    if (value == 1)
    return vec3(0, 1, 0);
    if (value == 2)
    return vec3(0, 0, 1);
    if (value == 3)
    return vec3(1, 1, 0);
    if (value == 4)
    return vec3(1, 0, 1);
    uint h = jenkinsHash(value);
    return (uvec3(h, h >> 8, h >> 16) & 0xffu) / 255.f;
}





vec2 get_screen_coordinate(vec3 world_pos){
    vec4 clip_pos = per_frame.view_proj * vec4(world_pos, 1.0);
    vec2 ndc_pos = clip_pos.xy / clip_pos.w;
    return (ndc_pos + 1.0) / 2.0;
}

float get_depth(vec3 world_pos){
    vec4 clip_pos = per_frame.view_proj * vec4(world_pos, 1.0);
    return clip_pos.z / clip_pos.w;
}


vec3 ssr(vec3 ori, vec3 dir) {
    dir = normalize(dir);
    vec3 hitPos;
    float step = 1.f;
    vec3 lastPoint = ori;
    //
    //  step = 0;
    //  return dir;
    for (int i=0;i<640;++i){
        // 往射线方向走一步得到测试点深度
        vec3 testPoint = lastPoint + step * dir;
        float testDepth = get_depth(testPoint);
        // 测试点的uv位置对应在depth buffer的深度
        vec2 testScreenUV = get_screen_coordinate(testPoint);
        testScreenUV.y = 1.0 - testScreenUV.y;
        float bufferDepth = texture(gbuffer_depth, testScreenUV).x;

        //   return texture(frame_color, testScreenUV).rgb;

        // 若测试点深度 > depth buffer深度，则说明光线相交于该测试点位置所在的像素柱条
        if (testDepth - bufferDepth   > 1e-4){
            hitPos = testPoint;
            //  return vec3(testDepth - bufferDepth);
            // return pseudocolor(i);
            return texture(frame_color, testScreenUV).rgb;
        }
        // 继续下一次 March
        lastPoint = testPoint;
    }
    return vec3(0.0);
}

uvec4 seed = init_rng(uvec2(gl_FragCoord.xy));

vec3 my_reflect(vec3 I, vec3 N){
    return -I + 2.0 * dot(I, N) * N;
}


void main(){

    vec4  diffuse_roughness  = subpassLoad(gbuffer_diffuse_roughness);
    vec4  normal_metalic    = subpassLoad(gbuffer_normal_metalic);
    vec3 normal      = normalize(2.0 * normal_metalic.xyz - 1.0);
    float metallic    = normal_metalic.w;

    vec3  emission = subpassLoad(gbuffer_emission).rgb;
    float depth    = texture(gbuffer_depth, vec2(in_uv.x, 1-in_uv.y)).x;
    //    //
    //    if (depth == 0.0){
    //        discard;
    //    }

    //    out_color = texture(frame_color, in_uv);
    //    out_color = vec4(1, 0.0, 0.0, 1.0);
    // out_color = vec4(depth, 0.0, 0.0, 1.0);
    //    return;

    vec3 world_pos = worldPosFromDepth(in_uv, depth);
    // return;


    vec3 diffuse_color = diffuse_roughness.xyz;
    float perceptual_roughness = diffuse_roughness.a;

    Frame frame = make_frame(normal);

    vec3 color = vec3(0.0);
    //unifrom sample 

    vec3 view_dir = per_frame.camera_pos - world_pos;
    vec3 reflect_dir = my_reflect(view_dir, normal);


    vec2 ray_end_screen = get_screen_coordinate(world_pos + reflect_dir);

    for (int i = 0; i < 10; ++i){
        vec3 dir = reflect_dir;
        //        dir = local_dir;
        //  color = dir;
        //  break;
        color += ssr(world_pos + 0.001 * dir, dir);
        // out_color = vec4(color, 1.0);
        //return;
    }
    // color = world_pos/10.f;
    //    color = reflect_dir * 10.f;
    out_color = vec4(color/10.f, 1.0);
}