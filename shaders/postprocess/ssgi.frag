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
    vec3 hitPos;
    float step = 1.0;
    vec3 lastPoint = ori;
    for (int i=0;i<10;++i){
        // 往射线方向走一步得到测试点深度
        vec3 testPoint = lastPoint + step * dir;
        float testDepth = get_depth(testPoint);
        // 测试点的uv位置对应在depth buffer的深度
        vec2 testScreenUV = get_screen_coordinate(testPoint);
        float bufferDepth = texture(gbuffer_depth, testScreenUV).x;
        // 若测试点深度 > depth buffer深度，则说明光线相交于该测试点位置所在的像素柱条
        if (testDepth-bufferDepth > -1e-6){
            hitPos = testPoint;
            return texture(frame_color, testScreenUV).rgb;
        }
        // 继续下一次 March
        lastPoint = testPoint;
    }
    return vec3(0.0);
}

uvec4 seed = init_rng(uvec2(gl_FragCoord.xy));


void main(){

    vec4  diffuse_roughness  = subpassLoad(gbuffer_diffuse_roughness);
    vec4  normal_metalic    = subpassLoad(gbuffer_normal_metalic);
    vec3 normal      = normalize(2.0 * normal_metalic.xyz - 1.0);
    float metallic    = normal_metalic.w;

    vec3  emission = subpassLoad(gbuffer_emission).rgb;
    float depth    = texture(gbuffer_depth, in_uv).x;

    if (depth == 0.0){
        discard;
    }

    vec3 world_pos = worldPosFromDepth(in_uv, depth);
    // return;


    vec3 diffuse_color = diffuse_roughness.xyz;
    float perceptual_roughness = diffuse_roughness.a;

    Frame frame = make_frame(normal);

    vec3 color = vec3(0.0);
    //unifrom sample 
    for (int i = 0; i < 10; ++i){
        vec2 rand = rand2(seed);
        vec3 local_dir = square_to_uniform_hemisphere(rand);
        vec3 dir = to_local(frame, local_dir);
        color += diffuse_color * ssr(world_pos + 0.001 * dir, dir);
    }
    out_color = vec4(color / 10.0, 1.0);
}