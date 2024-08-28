#version 460 core

#extension GL_GOOGLE_include_directive : enable 
#extension GL_EXT_debug_printf : enable


#include "perFrame.glsl"
//#include "../shadow.glsl"
#include "lighting.glsl"
#include "brdf.glsl"

precision highp float;

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_color;

layout(binding = 4, set = 0) uniform LightsInfo
{
    Light lights[MAX_LIGHTS];
}lights_info;




layout(input_attachment_index = 0, binding = 0, set=2) uniform subpassInput gbuffer_diffuse_roughness;
//layout(input_attachment_index = 1, binding = 1, set=2) uniform subpassInput gbuffer_specular;
layout(input_attachment_index = 1, binding = 1, set=2) uniform subpassInput gbuffer_normal_metalic;
layout(input_attachment_index = 2, binding = 2, set=2) uniform subpassInput gbuffer_emission;
layout(input_attachment_index = 3, binding = 3, set=2) uniform subpassInput gbuffer_depth;


void main(){
    //  return ;

    vec4  diffuse_roughness  = subpassLoad(gbuffer_diffuse_roughness);
    vec4  normal_metalic    = subpassLoad(gbuffer_normal_metalic);
    vec3 normal      = normalize(2.0 * normal_metalic.xyz - 1.0);
    float metallic    = normal_metalic.w;

    vec3  emission = subpassLoad(gbuffer_emission).rgb;
    float depth    = subpassLoad(gbuffer_depth).x;

    if (depth == 1.0){
        discard;
    }

    vec3 world_pos = worldPosFromDepth(in_uv, depth);
    // return;


    vec3 diffuse_color = diffuse_roughness.xyz;
    float perceptual_roughness = diffuse_roughness.a;

    vec3 view_dir = per_frame.camera_pos - world_pos;


    //calcuate sppecular contribution
    vec3 direct_contribution = vec3(0.0);

    bool has_emission = any(greaterThan(emission, vec3(1e-6)));
    if (has_emission)
    {
        direct_contribution+= emission;
    }
    else
    {
        // calculate Microfacet BRDF model
        // Roughness is authored as perceptual roughness; as is convention
        // convert to material roughness by squaring the perceptual roughness [2].
        // for 

        vec3 view_dir = normalize(per_frame.camera_pos - world_pos);

        PBRInfo pbr_info;
        // why use abs here?
        pbr_info.NdotV = clamp(abs(dot(normal, view_dir)), 0.001, 1.0);

        pbr_info.F0 = mix(vec3(0.04), diffuse_color, metallic);
        pbr_info.F90 = vec3(1.0);
        pbr_info.alphaRoughness = perceptual_roughness * perceptual_roughness;
        //  pbr_info.alphaRoughness = 0.01f;
        pbr_info.diffuseColor = diffuse_color;

        for (uint i = 0U; i < per_frame.light_count; ++i)
        {
            vec3 light_dir = calcuate_light_dir(lights_info.lights[i], world_pos);

            vec3 half_vector = normalize(light_dir + view_dir);

            pbr_info.NdotL = clamp(dot(normal, light_dir), 0.001, 1.0);
            pbr_info.NdotH = clamp(dot(normal, half_vector), 0.0, 1.0);
            pbr_info.LdotH = clamp(dot(light_dir, half_vector), 0.0, 1.0);
            pbr_info.VdotH = clamp(dot(view_dir, half_vector), 0.0, 1.0);

            vec3 light_contribution = microfacetBRDF(pbr_info) * apply_light(lights_info.lights[i], world_pos, normal) * calcute_shadow(lights_info.lights[i], world_pos);
            // light_contribution = 1-vec3(FresnelSchlick(pbr_info));
            //debugPrintfEXT("light_dir: %f %f %f\n", light_dir.x, light_dir.y, light_dir.z);
            //            direct_contribution =vec3(light_dir);

        }
    }
//    direct_contribution = vec3(1,0,0);
    out_color = vec4(direct_contribution, 1);
}