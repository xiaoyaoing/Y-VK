#version 460 core
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_debug_printf : enable
#extension GL_EXT_nonuniform_qualifier : enable

#include "shadow.glsl"
//#include "perFrameShading.glsl"
#include "perFrame.glsl"
#include "brdf.glsl"

precision highp float;

#define MIN_ROUGHNESS 0.04

layout (location = 0) in vec2 in_uv;
layout (location = 1) in vec3 in_normal;
layout (location = 2) flat in uint in_primitive_index;
layout (location = 3) in vec3 in_world_pos;

layout (location = 0) out vec4 o_color;

layout(std430, set = 0, binding = 2) buffer _GlobalPrimitiveUniform {
    PerPrimitive primitive_infos[];
};

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

layout(binding = 4, set = 0) uniform LightsInfo
{
    Light lights[MAX_LIGHTS];
}lights_info;

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


vec4 SRGBtoLinear(vec4 srgbIn, float gamma)
{
    return vec4(pow(srgbIn.xyz, vec3(gamma)), srgbIn.w);
}


vec3 getNormal(const int texture_idx)
{     
    vec3 tangentNormal = texture(scene_textures[texture_idx], in_uv).xyz * 2.0 - 1.0;

    vec3 q1 = dFdx(in_world_pos);
    vec3 q2 = dFdy(in_world_pos);
    vec2 st1 = dFdx(in_uv);
    vec2 st2 = dFdy(in_uv);

    vec3 N = normalize(in_normal);
    vec3 T = normalize(q1 * st2.t - q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    //return in_normal;
    return normalize(TBN * tangentNormal);
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
    
    return diffuse * 0.5f  + specular;
}




vec3 calculateShading(PBRInfo pbr_info,vec3 normal,vec3 R){

    vec3 color = vec3(0.0);
    
    vec3 view_dir = -normalize(per_frame.camera_pos - pbr_info.worldPos);
    {
     color += ibl_fragment_shader(pbr_info, normal, R);
    }
    
    {
        for (uint i = 0U; i < per_frame.light_count; ++i)
        {
            vec3 light_dir = calcuate_light_dir(lights_info.lights[i], pbr_info.worldPos);

            vec3 half_vector = normalize(light_dir + view_dir);

            pbr_info.NdotL = clamp(dot(normal, light_dir), 0.001, 1.0);
            pbr_info.NdotH = clamp(dot(normal, half_vector), 0.0, 1.0);
            pbr_info.LdotH = clamp(dot(light_dir, half_vector), 0.0, 1.0);
            pbr_info.VdotH = clamp(dot(view_dir, half_vector), 0.0, 1.0);

            vec3 light_contribution = microfacetBRDF(pbr_info) *   apply_light(lights_info.lights[i], pbr_info.worldPos, normal);
           // color += light_contribution;
        }
    }
    return   color * vec3(calcute_shadow(lights_info.lights[0], pbr_info.worldPos));
}


void main(void)
{
    
    vec4 diffuse_roughness;
    vec4 normal_metalic;
    vec4 emssion;

    uint material_index = primitive_infos[in_primitive_index].material_index;
    GltfMaterial material = scene_materials[material_index];

    vec3 diffuseColor            = vec3(0.0);
    vec3 specularColor            = vec3(0.0);
    vec4 baseColor                = vec4(0.0, 0.0, 0.0, 1.0);
    vec3 f0                        = vec3(0.04);
    float perceptual_roughness;
    float metallic;

    perceptual_roughness = material.pbrRoughnessFactor;
    
  
    
    
    metallic = material.pbrMetallicFactor;
    // Roughness is stored in the 'g' channel, metallic is stored in the 'b' channel
    // This layout intentionally reserves the 'r' channel for (optional) occlusion map data
    if (material.pbrMetallicRoughnessTexture > -1)
    {
        vec4 mrSample = texture(scene_textures[material.pbrMetallicRoughnessTexture], in_uv);
        perceptual_roughness *= mrSample.g;
        metallic *= mrSample.b;
    }
    else
    {
        perceptual_roughness = clamp(perceptual_roughness, MIN_ROUGHNESS, 1.0);
        metallic = clamp(metallic, 0.0, 1.0);
    }


    if(per_frame.use_roughness_override > 0.0)
    {
        perceptual_roughness = per_frame.roughness_override;
    }

    perceptual_roughness *= per_frame.roughness_scale;

  //  perceptual_roughness = 100000.f;

    baseColor = material.pbrBaseColorFactor;
    if (material.pbrBaseColorTexture > -1)
    {
        baseColor *= texture(scene_textures[material.pbrBaseColorTexture], in_uv);
    }
    diffuseColor = baseColor.rgb;

    diffuse_roughness  = vec4(diffuseColor, perceptual_roughness);

    vec3 normal = material.normalTexture > -1 ? getNormal(material.normalTexture) : normalize(in_normal);

    normal_metalic = vec4(normal * 0.5f  + 0.5f, metallic);
    vec3 emissionColor            = material.emissiveFactor;
    if (material.emissiveTexture > -1)
    {
        emissionColor *= SRGBtoLinear(texture(scene_textures[material.emissiveTexture], in_uv), 2.2).rgb;
    }
    emssion = vec4(emissionColor, 1.0);
    
    vec3 view_dir = -normalize(per_frame.camera_pos - in_world_pos);
    vec3 R = normalize(reflect(view_dir, normal));
    R.y = -R.y;
    
    PBRInfo pbr_info;
    pbr_info.NdotV = clamp(abs(dot(normal, view_dir)), 0.001, 1.0);
    pbr_info.F0 = mix(vec3(0.04), diffuseColor, metallic);
    pbr_info.F90 = vec3(1.0);
    pbr_info.alphaRoughness = perceptual_roughness * perceptual_roughness;
    pbr_info.perceptualRoughness = perceptual_roughness;
    pbr_info.diffuseColor = diffuseColor * (1- metallic) * (1-0.04);
    pbr_info.worldPos = in_world_pos;
    
    o_color = vec4(calculateShading(pbr_info, normal,R), baseColor.w);
}
