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

//layout(set = 1,binding = 5) uniform sampler2D blue_noise;

layout(std430, set = 0, binding = 2) buffer _GlobalPrimitiveUniform {
    PerPrimitive primitive_infos[];
};



vec4 SRGBtoLinear(vec4 srgbIn, float gamma)
{
    return vec4(pow(srgbIn.xyz, vec3(gamma)), srgbIn.w);
}

float MitchellCubic(float x) {
    const float B = 1.0f / 3.0f;
    const float C = 1.0f / 3.0f;
    float ax = abs(x);
    float ax2 = ax * ax;
    float ax3 = ax2 * ax;
    if(ax<1)
    return ((12.0f - 9.0f * B - 6.0f * C) * ax3 + (-18.0f + 12.0f * B + 6.0f * C) * ax2 + (6.0f - 2.0f * B)) / 6.0f;
    else 
    return ((-B - 6.0f * C) * ax3 + (6.0f * B + 30.0f * C) * ax2 + (-12.0f * B - 48.0f * C) * ax + (8.0f * B + 24.0f * C)) / 6.0f;
    }
vec3 SampleBicubic1(uint texture_idx,in vec2 uv,float u,float lod) {

    ivec2 size = textureSize(scene_point_textures[texture_idx],0);
    vec2 pixel_coord = uv * vec2(size);

    const vec2 top_left = floor(pixel_coord);

    //  return imageLoad(scene_storage_textures[texture_idx], ivec2(top_left)).xyz;
    const vec2 fract_offset = pixel_coord - top_left;
    float pos_weights_sum = 0.0f;
    float neg_weights_sum = 0.0f;
    vec2 selected_neg_offset;
    vec2 selected_pos_offset;
    for (int dy = -1; dy <= 2; ++dy) {
        float weight_dy = MitchellCubic(fract_offset.y - dy);
        for (int dx = -1; dx <= 2; ++dx) {
            float weight_dx = MitchellCubic(fract_offset.x - dx);
            float w = weight_dy * weight_dx;

            if(w<0.f)
            neg_weights_sum += abs(w);
            else
            pos_weights_sum += w;

            float selected_reservoir_sum = w < 0.0f ? neg_weights_sum : pos_weights_sum;

            float p = abs(w) / selected_reservoir_sum;
            if (u <= p) {
                if(w<0){
                    selected_neg_offset = vec2(dx, dy);
                }
                else{
                    selected_pos_offset = vec2(dx, dy);
                }
                u = u / p;
            } else {
                u = (u - p)/(1 - p);
            }
        }
    }
    vec2 pos_coord = top_left + selected_pos_offset + 0.5f;
    pos_coord /= vec2(size);
    vec3 sampled_val;
    if(lod!=-1)
        sampled_val  = pos_weights_sum * textureLod(scene_point_textures[texture_idx], pos_coord,lod).xyz;
    else  sampled_val  = pos_weights_sum * texture(scene_point_textures[texture_idx], pos_coord).xyz;
    // It's possible to not have any negative sample, for example,
    // when the fractional offset is exactly 0 or very small.
    if (neg_weights_sum != 0.0f) {
        vec2 neg_coord = top_left + selected_neg_offset;
        neg_coord /= vec2(size);
        if(lod!=-1) sampled_val += textureLod(scene_point_textures[texture_idx],neg_coord,lod).xyz * (-neg_weights_sum);
        sampled_val += texture(scene_point_textures[texture_idx],neg_coord).xyz * (-neg_weights_sum);
    }
    return sampled_val;
}

//vec3 SampleBicubic(uint texture_idx,in vec2 uv,float u) {
//
//    ivec2 size = imageSize(scene_storage_textures[texture_idx]);
//    vec2 pixel_coord = uv * vec2(size);
//
//    const vec2 top_left = floor(pixel_coord);
//    
//  //  return imageLoad(scene_storage_textures[texture_idx], ivec2(top_left)).xyz;
//    const vec2 fract_offset = pixel_coord - top_left;
//    float pos_weights_sum = 0.0f;
//    float neg_weights_sum = 0.0f;
//    vec2 selected_neg_offset;
//    vec2 selected_pos_offset;
//    for (int dy = -1; dy <= 2; ++dy) {
//        float weight_dy = MitchellCubic(fract_offset.y - dy);
//        for (int dx = -1; dx <= 2; ++dx) {
//            float weight_dx = MitchellCubic(fract_offset.x - dx);
//            float w = weight_dy * weight_dx;
//
//            if(w<0.f)
//            neg_weights_sum += abs(w);
//            else
//            pos_weights_sum += w;
//
//            float selected_reservoir_sum = w < 0.0f ? neg_weights_sum : pos_weights_sum;
//
//            float p = abs(w) / selected_reservoir_sum;
//            if (u <= p) {
//                if(w<0){
//                    selected_neg_offset = vec2(dx, dy);
//                }
//                else{
//                    selected_pos_offset = vec2(dx, dy);
//                }
//                u = u / p;
//            } else {
//                u = (u - p)/(1 - p);
//            }
//        }
//    }
//    vec2 pos_coord = top_left + selected_pos_offset;
//    vec3 sampled_val = pos_weights_sum *imageLoad(scene_storage_textures[texture_idx], ivec2(pos_coord)).xyz;
//    // It's possible to not have any negative sample, for example,
//    // when the fractional offset is exactly 0 or very small.
//    if (neg_weights_sum != 0.0f) {
//        vec2 neg_coord = top_left + selected_neg_offset;
//        sampled_val += imageLoad(scene_storage_textures[texture_idx], ivec2(neg_coord)).xyz * -neg_weights_sum;
//    }
//    return sampled_val;
//}



layout(push_constant) uniform PushConstant {
    uint  frame_index;
uint jitter;
    ivec2 screen_size;
    uint stochastic;
    uint padding[3];

} pc;

uvec4 init_rng(uvec2 pixel_coords, uint frame_num) {
    return uvec4(pixel_coords.xy, frame_num, 2);
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


vec3 stochastic_gauss_sample(uint tex_index,in vec2 uv,in vec2 rand) {
    vec2 orig_tex_coord = uv * textureSize(scene_textures[tex_index], 0) - 0.5;
    vec2 uv_full = (round(orig_tex_coord + boxMullerTransform(rand)*0.5)+0.5) / textureSize(scene_textures[tex_index], 0);
    return texture(scene_textures[tex_index], uv_full).xyz;
}

vec2 stochastic_bilin(in vec2 uv, in vec2 rand,ivec2 texture_size) {
    vec2 orig_tex_coord2 = uv * texture_size - 0.5;
    vec2 uv_full2 = (round(orig_tex_coord2 + rand - 0.5)+0.5) / texture_size;
    return uv_full2;
}

vec3 stochastic_bilin_sample(uint tex_index,in vec2 uv,in vec2 rand) {
    vec2 orig_tex_coord2 = uv * textureSize(scene_textures[tex_index], 0) - 0.5;
    vec2 uv_full2 = (round(orig_tex_coord2 + rand - 0.5)+0.5) / textureSize(scene_textures[tex_index], 0);
    return texture(scene_textures[tex_index], uv_full2).xyz;
}


//vec3 stochastic(int tex_index,in vec2 uv,in bool need_stochastic,float u) {
//    if (need_stochastic) {
//        return SampleBicubic1(tex_index,uv,u);
//    }
//    else {
//        return texture(scene_textures[tex_index], uv).xyz;
//    }
//}

float computeLodAniso(vec2 uv, float minLod, float maxLod, float u)
{
    float dudx = dFdx(uv.x);
    float dudy = dFdy(uv.x);
    float dvdx = dFdx(uv.y);
    float dvdy = dFdy(uv.y);

    // Find min and max ellipse axis
    vec2 maxAxis = vec2(dudy, dvdy);
    vec2 minAxis = vec2(dudx, dvdx);
    if (dot(minAxis, minAxis) > dot(maxAxis, maxAxis))
    {
        minAxis = vec2(dudy, dvdy);
        maxAxis = vec2(dudx, dvdx);
    }
    float minAxisLength = length(minAxis);
    float maxAxisLength = length(maxAxis);
    float maxAnisotropy = 64;
    if ( minAxisLength > 0 &&
    (minAxisLength * maxAnisotropy) < maxAxisLength)
    {
        float scale = maxAxisLength / (minAxisLength * maxAnisotropy);
        minAxisLength *= scale;
    }
    return clamp(log2(minAxisLength) + (u - 0.5), minLod, maxLod);
}

vec3 stochastic_sample(uint tex_index,in vec2 uv,bool need_stochastic,inout uvec4 seed) {
    float aniso = computeLodAniso(uv, 0.0, 13, rand1(seed));

    if (need_stochastic) {
        //return stochastic_gauss_sample(tex_index,uv,rand2(seed));
      //  return stochastic_gauss_sample(tex_index,uv,rand2(seed));
        return SampleBicubic1(tex_index,uv,rand1(seed),-1);
    }
    else {
        return texture(scene_textures[tex_index], uv).xyz;
    }

}



vec3 getNormal(const int texture_idx, vec2 uv,inout uvec4 seed,bool need_stochastic)
{
    //    texture_idx = clamp(texture_idx, 0, 136);
    //    return texture(scene_textures[texture_idx], in_uv).xyz * 2.0 - 1.0;
    //    return vec3(0);
    // Perturb normal, see http://www.thetenthplanet.de/archives/1180
  //  vec2 lodInfo = textureQueryLod(scene_textures[texture_idx], uv);
    
    vec3 tangentNormal;
    tangentNormal = stochastic_sample(texture_idx,uv,need_stochastic,seed) * 2.0 - 1.0;

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

void main(void)
{

    vec2 uv = fract(in_uv * 5.f) ;//+ 0.2f;\
    uv = clamp(uv,vec2(0.001),vec2(0.999));
  //  uv = in_uv;
    //    return;
    uint material_index = primitive_infos[in_primitive_index].material_index;
    GltfMaterial material = scene_materials[material_index];
    
    bool need_stochastic = gl_FragCoord.x < pc.screen_size.x * 0.5f;
    if(pc.stochastic != 0 ){
        need_stochastic = pc.stochastic == 1;
    }

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
      //  vec2 sample_uv = stochastic(material.pbrMetallicRoughnessTexture,uv, need_stochastic,rand1(seed),need_stochastic);
        vec3 mrSample =stochastic_sample(material.pbrMetallicRoughnessTexture,uv,need_stochastic,seed);
        perceptualRoughness = mrSample.g;
        metallic = mrSample.b;
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
        baseColor.xyz *= stochastic_sample(material.pbrBaseColorTexture,uv,need_stochastic,seed);

//        ivec2 uvI = ivec2(sample_uv * vec2(20.f));
//        bool  on = bool((uvI.x ^ uvI.y) & 1);
      //  baseColor = on?vec4(1,1,1,1):vec4(0,0,0,1);
    }
    diffuseColor = baseColor.rgb;
   // diffuseColor = vec3(uv, 0.0);

    o_diffuse_roughness  = vec4(diffuseColor, perceptualRoughness);

    vec3 normal = material.normalTexture > -1 ? getNormal(material.normalTexture,uv,seed,need_stochastic) : normalize(in_normal);

    o_normal_metalic = vec4(normal * 0.5f  + 0.5f, metallic);
    //debugPrintfEXT("metallic: %f\n",metallic);
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
    
    if(o_diffuse_roughness.xyz == vec3(0)){
        
       // debugPrintfEXT("uv: %f %f\n",uv.x,uv.y);
       // o_diffuse_roughness.xyz = vec3(uv,0);
    }

    //o_normal_metalic.xyz = vec3(rand1(seed),rand1(seed),rand1(seed));
//o_diffuse_roughness.xy = uv;
//    o_diffuse_roughness.z = 0;
   // o_diffuse_roughness.xyz = vec3(gl_FragCoord.xy/vec2(1920.f,1080.f),0);
  //  o_diffuse_roughness.xyz = texture(blue_noise, uv).xyz;
   // debugPrintfEXT("gl_fragcoord.x: screen_size.x: %f %f p: %f\n",gl_FragCoord.x,pc.screen_size.x,p);
    //o_diffuse_roughness.xyz = vec3(uv,0);
    //debugPrintfEXT("screen_size.x : %f",pc.screen_size.x);
    o_emssion = vec4(emissionColor, 1.0);
}
