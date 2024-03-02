#version 460 core
#extension GL_GOOGLE_include_directive : enable
#include "shadow.glsl"
#include "perFrameShading.glsl"

precision highp float;

// #ifdef HAS_baseColorTexture
//layout (set=1, binding=0) uniform sampler2D baseColorTexture;
// #endif
#define MIN_ROUGHNESS 0.04


layout (location = 0) in vec2 in_uv;
layout (location = 1) in vec3 in_normal;

layout (location = 0) out vec4 o_diffuse;
layout (location = 1) out vec4 o_specular;
layout (location = 2) out vec4 o_normal;
layout (location = 3) out vec4 o_emssion;

layout(push_constant) uniform PushConstantBlock {
    uint u_materialIndex;
};

vec4 SRGBtoLinear(vec4 srgbIn, float gamma)
{
    return vec4(pow(srgbIn.xyz, vec3(gamma)), srgbIn.w);
}

void main(void)
{
    vec3 normal = normalize(in_normal);
    // Transform normals from [-1, 1] to [0, 1]
    o_normal = vec4(0.5 * normal + 0.5, 1.0);

    GltfMaterial material = scene_materials[u_materialIndex];

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
    specularColor = mix(f0, baseColor.rgb, metallic);

    if (material.alphaMode > 0 && baseColor.a < material.alphaCutoff)
    {
        //        discard;
    }

    o_diffuse  = vec4(diffuseColor, perceptualRoughness);
    o_specular = vec4(specularColor, metallic);
    // gbufferNormal = vec4(normalize(fs_in.normal) * 0.5 + 0.5, 1.0);

    //    vec4 tangent = vec4(normalize(fs_in.tangent.xyz), fs_in.tangent.w);
    //    gbufferTangent = vec4(tangent * 0.5 + 0.5);

    vec3 emissionColor            = material.emissiveFactor;
    if (material.emissiveTexture > -1)
    {
        emissionColor *= SRGBtoLinear(texture(scene_textures[material.emissiveTexture], in_uv), 2.2).rgb;
    }
    o_emssion = vec4(emissionColor, 1.0);
    //    o_normal =  o_albedo;


}
