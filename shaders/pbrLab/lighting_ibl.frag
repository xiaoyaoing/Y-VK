#version 460 core

#extension GL_GOOGLE_include_directive : enable 
#extension GL_EXT_debug_printf : enable


#include "../perFrame.glsl"
//#include "../shadow.glsl"
#include "../lighting.glsl"
#include "../brdf.glsl"


//    ImGui::Combo("Debug Mode", &debugMode, "None\0Diffuse\0Specular\0Normal\0Depth\0Albedo\0Metallic\0Roughness\0Ambient Occlusion\0Irradiance\0Prefilter\0BRDF LUT\0");

#define DEBUG_MODEL_DIFFUSE 1
#define DEBUG_MODEL_SPECULAR 2
#define DEBUG_MODEL_NORMAL 3
#define DEBUG_MODEL_DEPTH 4
#define DEBUG_MODEL_ALBEDO 5
#define DEBUG_MODEL_METALLIC 6
#define DEBUG_MODEL_ROUGHNESS 7
#define DEBUG_MODEL_AMBIENT_OCCLUSION 8
#define DEBUG_MODEL_IRRADIANCE 9
#define DEBUG_MODEL_PREFILTER 10
#define DEBUG_MODEL_BRDF_LUT 11


precision highp float;

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_color;


#define ROUGHNESS_PREFILTER_COUNT 5

layout(binding = 4, set = 0) uniform LightsInfo
{
    Light lights[MAX_LIGHTS];
}lights_info;


layout(input_attachment_index = 0, binding = 0, set=2) uniform subpassInput gbuffer_diffuse_roughness;
//layout(input_attachment_index = 1, binding = 1, set=2) uniform subpassInput gbuffer_specular;
layout(input_attachment_index = 1, binding = 1, set=2) uniform subpassInput gbuffer_normal_metalic;
layout(input_attachment_index = 2, binding = 2, set=2) uniform subpassInput gbuffer_emission;
layout(input_attachment_index = 3, binding = 3, set=2) uniform subpassInput gbuffer_depth;

layout(binding = 2, set=1) uniform sampler2D brdf_lut;
layout(binding = 0, set=1) uniform samplerCube irradiance_map;
layout(binding = 1, set=1) uniform samplerCube prefilter_map;

layout(push_constant) uniform Params
{
    float exposure;
    float gamma;
    float scaleIBLAmbient;
    float prefilteredCubeMipLevels;
    int debugMode;
    int padding[3];
};

vec3 Uncharted2Tonemap(vec3 color)
{
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    float W = 11.2;
    return ((color*(A*color+C*B)+D*E)/(color*(A*color+B)+D*F))-E/F;
}


vec4 tonemap(vec4 color)
{
    vec3 outcol = Uncharted2Tonemap(color.rgb * exposure);
    outcol = outcol * (1.0f / Uncharted2Tonemap(vec3(11.2f)));
    return vec4(pow(outcol, vec3(1.0f / gamma)), color.a);
}

vec4 SRGBtoLINEAR(vec4 srgbIn)
{
    vec3 bLess = step(vec3(0.04045), srgbIn.xyz);
    vec3 linOut = mix(srgbIn.xyz/vec3(12.92), pow((srgbIn.xyz+vec3(0.055))/vec3(1.055), vec3(2.4)), bLess);
    return vec4(linOut, srgbIn.w);;
}

vec3 ibl_fragment_shader(const in PBRInfo pbr_info, vec3 n, vec3 reflection)
{
    float lod = (pbr_info.perceptualRoughness * prefilteredCubeMipLevels);
    vec3 brdf = (texture(brdf_lut, vec2(pbr_info.NdotV, pbr_info.perceptualRoughness))).rgb;
    vec3 diffuseLight = SRGBtoLINEAR(tonemap(texture(irradiance_map, n))).rgb;

    vec3 specularLight = SRGBtoLINEAR(tonemap(textureLod(prefilter_map, reflection, lod))).rgb;
    vec3 diffuse = diffuseLight * pbr_info.diffuseColor;
    vec3 specular = specularLight * (pbr_info.F0 * brdf.x + brdf.y);

    // return pbr_info.F0;
    // return vec3(brdf.y);
    // For presentation, this allows us to disable IBL terms
    // For presentation, this allows us to disable IBL terms
    
    diffuse *= scaleIBLAmbient;
    specular *= scaleIBLAmbient;

    if(debugMode == DEBUG_MODEL_DIFFUSE)
        return diffuseLight;
    if(debugMode == DEBUG_MODEL_SPECULAR)
        return specularLight;
    return diffuse + specular;
}


void main(){
    //  return;
    vec4  diffuse_roughness  = subpassLoad(gbuffer_diffuse_roughness);
    vec4  normal_metalic    = subpassLoad(gbuffer_normal_metalic);

    vec3 normal      = normalize(2.0 * normal_metalic.xyz - 1.0);
    float metallic    = normal_metalic.w;

    vec3  emission = subpassLoad(gbuffer_emission).rgb;
    float depth    = subpassLoad(gbuffer_depth).x;

    if (depth == 1.0){
        discard;
    }

    vec3 world_pos = worldPosFromDepth(vec2(in_uv.x,in_uv.y), depth);


    vec3 diffuse_color = diffuse_roughness.xyz;
    float perceptual_roughness = diffuse_roughness.a;

    vec3 view_dir = per_frame.camera_pos - world_pos;
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

        vec3 view_dir = -normalize(per_frame.camera_pos - world_pos);
        vec3 R = normalize(reflect(view_dir, normal));
        R.y = -R.y;
        PBRInfo pbr_info;
        // why use abs here?
        pbr_info.NdotV = clamp(abs(dot(normal, view_dir)), 0.001, 1.0);
                            
        pbr_info.F0 = mix(vec3(0.04), diffuse_color, metallic);
        pbr_info.F90 = vec3(1.0);
        pbr_info.alphaRoughness = perceptual_roughness * perceptual_roughness;
        pbr_info.perceptualRoughness = perceptual_roughness;
        pbr_info.diffuseColor = diffuse_color * (1- metallic) * (1-0.04);

         color += ibl_fragment_shader(pbr_info, normal, R);
    }

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

            vec3 light_contribution = microfacetBRDF(pbr_info) *  calcute_shadow(lights_info.lights[i], world_pos) * apply_light(lights_info.lights[i], world_pos, normal);
            //            light_contribution = diffuse(pbr_info);
            //debugPrintfEXT("light_contribution: %f %f %f\n", light_contribution.x, light_contribution.y, light_contribution.z);
            color += light_contribution;
        }
    }

    
    if (debugMode == DEBUG_MODEL_NORMAL)
    {
        out_color = vec4(normal         , 1);
        return;
    }
    if (debugMode == DEBUG_MODEL_DEPTH)
    {
        out_color = vec4(vec3(depth), 1);
        return;
    }
    if (debugMode == DEBUG_MODEL_ALBEDO)
    {
        out_color = vec4(diffuse_color, 1);
        return;
    }
    if (debugMode == DEBUG_MODEL_METALLIC)
    {
        out_color = vec4(vec3(metallic), 1);
        return;
    }
    if (debugMode == DEBUG_MODEL_ROUGHNESS)
    {
        out_color = vec4(vec3(perceptual_roughness), 1);
        return;
    }
    if (debugMode == DEBUG_MODEL_AMBIENT_OCCLUSION)
    {
        out_color = vec4(vec3(1.0), 1);
        return;
    }
    if (debugMode == DEBUG_MODEL_IRRADIANCE)
    {
        out_color = vec4(vec3(1.0), 1);
        return;
    }
    if (debugMode == DEBUG_MODEL_PREFILTER)
    {
        out_color = vec4(vec3(1.0), 1);
        return;
    }
    
    //color = world_pos / 100.f;
    out_color = vec4(color, 1);
}