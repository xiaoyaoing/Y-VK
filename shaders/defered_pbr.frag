#version 460 core
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_debug_printf : enable
#extension GL_EXT_nonuniform_qualifier : enable

//#include "shadow.glsl"
#ifdef DIRECT_LIGHTING
#include "shadow.glsl"
#include "brdf.glsl"
#else
#include "perFrameShading.glsl"
#endif

#include "perFrame.glsl"

precision highp float;

#define MIN_ROUGHNESS 0.04


layout (location = 0) in vec2 in_uv;
layout (location = 1) in vec3 in_normal;
layout (location = 2) flat in uint in_primitive_index;
layout (location = 3) in vec3 in_world_pos;

#ifndef OUTPUT_TO_BUFFER
layout (location = 0) out vec4 o_diffuse_roughness;
layout (location = 1) out vec4 o_normal_metalic;
layout (location = 2) out vec4 o_emssion;
#else
struct GBuffer {
    vec3 position;
    uint material_idx;
    vec2 uv;
    vec3 normal;
};
layout(std430, binding = 5) buffer _GBuffer {
    GBuffer d[];
} gbuffer;
#endif

#ifdef DIRECT_LIGHTING
layout(binding = 4) uniform LightsInfo
{
    Light lights[MAX_LIGHTS];
}lights_info;
#ifndef OUTPUT_TO_BUFFER
layout (location = 3) out vec4 o_color;
#else
layout(location = 0) out vec4 o_color;
#endif 

#endif


layout(std430, set = 0, binding = 2) buffer _GlobalPrimitiveUniform {
    PerPrimitive primitive_infos[];
};



vec4 SRGBtoLinear(vec4 srgbIn, float gamma)
{
    return vec4(pow(srgbIn.xyz, vec3(gamma)), srgbIn.w);
}


vec3 getNormal(const int texture_idx)
{
    if (texture_idx <0){
        return normalize(in_normal);
    }
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


void main(void)
{

    uint material_index = primitive_infos[in_primitive_index].material_index;
    GltfMaterial material = scene_materials[material_index];

    vec3 diffuse_color            = vec3(0.0);
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

    if (per_frame.use_roughness_override > 0.0)
    {
        perceptual_roughness = per_frame.roughness_override;
    }

    perceptual_roughness *= per_frame.roughness_scale;

    baseColor = material.pbrBaseColorFactor;
    if (material.pbrBaseColorTexture > -1)
    {
        baseColor *= texture(scene_textures[material.pbrBaseColorTexture], in_uv);
    }
    diffuse_color = baseColor.rgb;


    vec3 normal = getNormal(material.normalTexture);

    vec3 emissionColor            = material.emissiveFactor;
    if (material.emissiveTexture > -1)
    {
        emissionColor *= SRGBtoLinear(texture(scene_textures[material.emissiveTexture], in_uv), 2.2).rgb;
    }


    #ifndef OUTPUT_TO_BUFFER
    o_diffuse_roughness  = vec4(diffuse_color, perceptual_roughness);
    o_normal_metalic = vec4(normal * 0.5f  + 0.5f, metallic);
    o_emssion = vec4(emissionColor, 1.0);
    #else
    uint g_buffer_index = uint(gl_FragCoord.y * per_frame.resolution.x + gl_FragCoord.x);
    gbuffer.d[g_buffer_index].position = in_world_pos;
    gbuffer.d[g_buffer_index].material_idx = material_index;
    gbuffer.d[g_buffer_index].uv = in_uv;
    gbuffer.d[g_buffer_index].normal = normal;
    #endif
    
    #ifdef DIRECT_LIGHTING
    vec3 view_dir = normalize(per_frame.camera_pos - in_world_pos);

    PBRInfo pbr_info;
    // why use abs here?
    pbr_info.NdotV = clamp(abs(dot(normal, view_dir)), 0.001, 1.0);

    pbr_info.F0 = mix(vec3(0.04), diffuse_color, metallic);
    pbr_info.F90 = vec3(1.0);
    pbr_info.alphaRoughness = perceptual_roughness * perceptual_roughness;
    //  pbr_info.alphaRoughness = 0.01f;
    pbr_info.diffuseColor = diffuse_color;

    vec3 light_contribution = vec3(0.0);
    for (uint i = 0U; i < per_frame.light_count; ++i)
    {
        vec3 light_dir = calcuate_light_dir(lights_info.lights[i], in_world_pos);

        vec3 half_vector = normalize(light_dir + view_dir);

        pbr_info.NdotL = clamp(dot(normal, light_dir), 0.001, 1.0);
        pbr_info.NdotH = clamp(dot(normal, half_vector), 0.0, 1.0);
        pbr_info.LdotH = clamp(dot(light_dir, half_vector), 0.0, 1.0);
        pbr_info.VdotH = clamp(dot(view_dir, half_vector), 0.0, 1.0);

        light_contribution = light_contribution + microfacetBRDF(pbr_info) * apply_light(lights_info.lights[i], in_world_pos, normal) * calcute_shadow(lights_info.lights[i], in_world_pos);
    }
    o_color = vec4(light_contribution, 1.0);
    #endif
}
