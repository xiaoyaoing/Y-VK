#version 460 core
#extension GL_GOOGLE_include_directive : enable
#include "shadow.glsl"
#include "perFrameShading.glsl"
#include "perFrame.glsl"

precision highp float;

#define MIN_ROUGHNESS 0.04


layout (location = 0) in vec2 in_uv;
layout (location = 1) in vec3 in_normal;
layout (location = 2) flat in uint in_primitive_index;


layout (location = 0) out vec4 o_diffuse_roughness;
layout (location = 1) out vec4 o_normal_metalic;
layout (location = 2) out vec4 o_emssion;


layout(std430, set = 0, binding = 2) buffer _GlobalPrimitiveUniform {
    PerPrimitive primitive_infos[];
};



vec4 SRGBtoLinear(vec4 srgbIn, float gamma)
{
    return vec4(pow(srgbIn.xyz, vec3(gamma)), srgbIn.w);
}

void main(void)
{
    vec3 normal = normalize(in_normal);


    uint material_index = primitive_infos[in_primitive_index].material_index;
    GltfMaterial material = scene_materials[material_index];

    vec3 diffuseColor            = vec3(0.0);
    vec3 specularColor            = vec3(0.0);
    vec4 baseColor                = vec4(0.0, 0.0, 0.0, 1.0);
    vec3 f0                        = vec3(0.04);
    float perceptualRoughness;
    float metallic;

    perceptualRoughness = material.pbrRoughnessFactor;
    metallic = material.pbrMetallicFactor;
    // Roughness is stored in the 'g' channel, metallic is stored in the 'b' channel
    // This layout intentionally reserves the 'r' channel for (optional) occlusion map data
    if (material.pbrMetallicRoughnessTexture > -1)
    {
        vec4 mrSample = texture(scene_textures[material.pbrMetallicRoughnessTexture], in_uv);
        perceptualRoughness *= mrSample.g;
        metallic *= mrSample.b;
    }
    else
    {
        perceptualRoughness = clamp(perceptualRoughness, MIN_ROUGHNESS, 1.0);
        metallic = clamp(metallic, 0.0, 1.0);
    }

    baseColor = material.pbrBaseColorFactor;
    if (material.pbrBaseColorTexture > -1)
    {
        baseColor *= texture(scene_textures[material.pbrBaseColorTexture], in_uv);
    }
    diffuseColor = baseColor.rgb * (vec3(1.0) - f0) * (1.0 - metallic);


    o_diffuse_roughness  = vec4(diffuseColor, perceptualRoughness);
    o_normal_metalic = vec4(normal * 0.5f  + 0.5f, metallic);
    vec3 emissionColor            = material.emissiveFactor;
    if (material.emissiveTexture > -1)
    {
        emissionColor *= SRGBtoLinear(texture(scene_textures[material.emissiveTexture], in_uv), 2.2).rgb;
    }
    o_emssion = vec4(emissionColor, 1.0);


}
