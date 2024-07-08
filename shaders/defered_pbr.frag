#version 460 core
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_debug_printf : enable
#extension GL_EXT_nonuniform_qualifier : enable

#include "shadow.glsl"
#include "perFrameShading.glsl"
#include "perFrame.glsl"

precision highp float;

#define MIN_ROUGHNESS 0.04


layout (location = 0) in vec2 in_uv;
layout (location = 1) in vec3 in_normal;
layout (location = 2) flat in uint in_primitive_index;
layout (location = 3) in vec3 in_world_pos;


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


vec3 getNormal(const int texture_idx, vec2 uv)
{
    //    texture_idx = clamp(texture_idx, 0, 136);
    //    return texture(scene_textures[texture_idx], in_uv).xyz * 2.0 - 1.0;
    //    return vec3(0);
    // Perturb normal, see http://www.thetenthplanet.de/archives/1180
    vec3 tangentNormal = texture(scene_textures[texture_idx], uv).xyz * 2.0 - 1.0;

    vec3 q1 = dFdx(in_world_pos);
    vec3 q2 = dFdy(in_world_pos);
    vec2 st1 = dFdx(uv);
    vec2 st2 = dFdy(uv);

    vec3 N = normalize(in_normal);
    vec3 T = normalize(q1 * st2.t - q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

layout(push_constant) uniform PushConstant {
    uint  frame_index;
        uint pad;
    ivec2 screen_size;
} pc;

uvec4 init_rng(uvec2 pixel_coords, uint frame_num) {
    return uvec4(pixel_coords.xy, frame_num, 0);
}


float uint_to_float(uint x) {
    return uintBitsToFloat(0x3f800000 | (x >> 9)) - 1.0f;
}

uvec4 pcg4d(uvec4 v) {
    v = v * 1664525u + 1013904223u;
    v.x += v.y * v.w;
    v.y += v.z * v.x;
    v.z += v.x * v.y;
    v.w += v.y * v.z;
    v = v ^ (v >> 16u);
    v.x += v.y * v.w;
    v.y += v.z * v.x;
    v.z += v.x * v.y;
    v.w += v.y * v.z;
    return v;
}

float rand1(inout uvec4 rng_state) {
    rng_state.w++;
    return uint_to_float(pcg4d(rng_state).x);
}

vec2 rand2(inout uvec4 rng_state) {
    rng_state.w++;
    uvec4 pcg = pcg4d(rng_state);
    return vec2(uint_to_float(pcg.x), uint_to_float(pcg.y));
}

#define PI 3.1415926

vec2 boxMullerTransform(vec2 u)
{
    vec2 r;
    float mag = sqrt(-2.0 * log(u.x));
    return mag * vec2(cos(2.0 * PI * u.y), sin(2.0 * PI * u.y));
}

vec2 stochastic_gauss(in vec2 uv, in vec2 rand,ivec2 texture_size) {
    vec2 orig_tex_coord = uv * texture_size - 0.5;
    vec2 uv_full = (round(orig_tex_coord + boxMullerTransform(rand)*0.5)+0.5) / texture_size;

    return uv_full;
}


vec2 stochastic_bilin(in vec2 uv, in vec2 rand,ivec2 texture_size) {
    vec2 orig_tex_coord2 = uv * texture_size - 0.5;
    vec2 uv_full2 = (round(orig_tex_coord2 + rand - 0.5)+0.5) / texture_size;
    return uv_full2;
}

vec2 stochastic(in vec2 uv, in ivec2 size, in bool need_stochastic,inout uvec4 seed) {
    if (need_stochastic) {
        return stochastic_gauss(uv, rand2(seed),size);
    }
    else {
        return uv;
    }
}

void main(void)
{

    vec2 uv = fract(in_uv * 20.f);
    //    return;
    uint material_index = primitive_infos[in_primitive_index].material_index;
    GltfMaterial material = scene_materials[material_index];
    
    bool need_stochastic = gl_FragCoord.x < pc.screen_size.x * 0.5f;

    vec3 diffuseColor            = vec3(0.0);
    vec3 specularColor            = vec3(0.0);
    vec4 baseColor                = vec4(0.0, 0.0, 0.0, 1.0);
    vec3 f0                        = vec3(0.04);
    float perceptualRoughness;
    float metallic;
    
    uvec4 seed = init_rng(uvec2(gl_FragCoord.xy),  pc.frame_index);

    perceptualRoughness = material.pbrRoughnessFactor;
    metallic = material.pbrMetallicFactor;
    // Roughness is stored in the 'g' channel, metallic is stored in the 'b' channel
    // This layout intentionally reserves the 'r' channel for (optional) occlusion map data
    if (material.pbrMetallicRoughnessTexture > -1)
    {
        ivec2 size = textureSize(scene_textures[material.pbrMetallicRoughnessTexture], 0);
        vec2 sample_uv = stochastic(uv, size, need_stochastic,seed);
        vec4 mrSample = texture(scene_textures[material.pbrMetallicRoughnessTexture], sample_uv,0);
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
        ivec2 size = textureSize(scene_textures[material.pbrBaseColorTexture], 0);
        vec2 sample_uv = stochastic(uv, size, need_stochastic,seed);
        baseColor *= texture(scene_textures[material.pbrBaseColorTexture], sample_uv,0);
    }
    diffuseColor = baseColor.rgb;
   // diffuseColor = vec3(uv, 0.0);

    o_diffuse_roughness  = vec4(diffuseColor, perceptualRoughness);

    vec3 normal = material.normalTexture > -1 ? getNormal(material.normalTexture,uv) : normalize(in_normal);

    o_normal_metalic = vec4(normal * 0.5f  + 0.5f, metallic);
    vec3 emissionColor            = material.emissiveFactor;
    if (material.emissiveTexture > -1)
    {
        emissionColor *= SRGBtoLinear(texture(scene_textures[material.emissiveTexture], uv), 2.2).rgb;
    }
    float p = gl_FragCoord.x / pc.screen_size.x;
    if(need_stochastic){
       // o_diffuse_roughness.xyz = vec3(rand2(seed),0);
    }
    if(abs(p-0.5f)<0.01){
        o_diffuse_roughness.xyz = vec3(1,0,0);
    }
    
   // debugPrintfEXT("gl_fragcoord.x: screen_size.x: %f %f p: %f\n",gl_FragCoord.x,pc.screen_size.x,p);
      //     o_diffuse_roughness.xyz = vec3(p,0,0);
    //debugPrintfEXT("screen_size.x : %f",pc.screen_size.x);
    o_emssion = vec4(emissionColor, 1.0);
}
