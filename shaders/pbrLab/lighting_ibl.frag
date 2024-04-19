#version 460 core

#extension GL_GOOGLE_include_directive : enable 
#extension GL_EXT_debug_printf : enable


#include "../perFrame.glsl"
//#include "../shadow.glsl"
#include "../lighting.glsl"
#include "../brdf.glsl"

precision highp float;

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_color;

layout(binding = 4, set = 0) uniform LightsInfo
{
    Light lights[MAX_LIGHTS];
}lights_info;


#define ROUGHNESS_PREFILTER_COUNT 5

layout(input_attachment_index = 0, binding = 0, set=2) uniform subpassInput gbuffer_diffuse_roughness;
//layout(input_attachment_index = 1, binding = 1, set=2) uniform subpassInput gbuffer_specular;
layout(input_attachment_index = 1, binding = 1, set=2) uniform subpassInput gbuffer_normal_metalic;
layout(input_attachment_index = 2, binding = 2, set=2) uniform subpassInput gbuffer_emission;
layout(input_attachment_index = 3, binding = 3, set=2) uniform subpassInput gbuffer_depth;

layout(binding = 0, set=1, rgba8) uniform sampler2D brdf_lut;
layout(binding = 1, set=1, rgba8) uniform samplerCube irradiance_map;
layout(binding = 2, set=1, rgba8) uniform samplerCube prefilter_map;

layout(push_constant) uniform Params
{
    float scaleIBLAmbient;
    float prefilteredCubeMipLevels;
};

vec3 ibl_fragment_shader(const PBRInfo pbr_info, vec3 n, vec3 reflection)
{
    float lod = (pbrInputs.alphaRoughness * prefilteredCubeMipLevels);
    // retrieve a scale and bias to F0. See [1], Figure 3
    vec3 brdf = (texture(samplerBRDFLUT, vec2(pbrInputs.NdotV, 1.0 - pbrInputs.perceptualRoughness))).rgb;
    vec3 diffuseLight = SRGBtoLINEAR(tonemap(texture(samplerIrradiance, n))).rgb;

    vec3 specularLight = SRGBtoLINEAR(tonemap(textureLod(prefilteredMap, reflection, lod))).rgb;

    vec3 diffuse = diffuseLight * pbrInputs.diffuseColor;
    vec3 specular = specularLight * (pbrInputs.F0 * brdf.x + brdf.y);

    // For presentation, this allows us to disable IBL terms
    // For presentation, this allows us to disable IBL terms
    diffuse *= scaleIBLAmbient;
    specular *= scaleIBLAmbient;

    return diffuse + specular;
}


void main(){
    vec4  diffuse_roughness  = subpassLoad(gbuffer_diffuse_roughness);
    vec4  normal_metalic    = subpassLoad(gbuffer_normal_metalic);
    vec3 normal      = normalize(2.0 * normal_metalic.xyz - 1.0);
    float metallic    = normal_metalic.w;

    vec3  emission = subpassLoad(gbuffer_emission).rgb;
    float depth    = subpassLoad(gbuffer_depth).x;

    vec3 world_pos = worldPosFromDepth(in_uv, depth);


    vec3 diffuse_color = diffuse_roughness.xyz;
    float perceptual_roughness = diffuse_roughness.a;

    vec3 view_dir = per_frame.camera_pos - world_pos;


    //calcuate sppecular contribution
    vec3 color = vec3(0.0);

    bool has_emission = any(greaterThan(emission, vec3(1e-6)));
    if (has_emission)
    {
        color+= emission;
    }

    {
        // calculate Microfacet BRDF model
        // Roughness is authored as perceptual roughness; as is convention
        // convert to material roughness by squaring the perceptual roughness [2].
        // for 

        vec3 view_dir = normalize(per_frame.camera_pos - world_pos);
        vec3 R = 2 * dot(normal, view_dir) * normal - view_dir;

        PBRInfo pbr_info;
        // why use abs here?
        pbr_info.NdotV = clamp(abs(dot(normal, view_dir)), 0.001, 1.0);

        pbr_info.F0 = mix(vec3(0.04), diffuse_color, metallic);
        pbr_info.F90 = vec3(1.0);
        pbr_info.alphaRoughness = perceptual_roughness * perceptual_roughness;
        //  pbr_info.alphaRoughness = 0.01f;
        pbr_info.diffuseColor = diffuse_color;

        color += ibl_fragment_shader(pbr_info, normal, R);
    }
    out_color = vec4(color, 1);
}