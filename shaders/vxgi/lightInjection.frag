#version 450
#extension GL_GOOGLE_include_directive : require
//#extension GLSL_EXT_shader_image_int64 : require
#extension GL_EXT_debug_printf : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_shader_image_load_store : require

precision mediump float;


// #include "vxgi_common.h"
#include "../PerFrame.glsl"
#include "vxgi.glsl"
#include "../shadow.glsl"
#include "../brdf.glsl"
layout(input_attachment_index = 0, binding = 0, set=2) uniform subpassInput gbuffer_diffuse_roughness;





layout(std430, set = 0, binding = 2) readonly buffer _GlobalPrimitiveUniform {
    PerPrimitive primitive_infos[];
};



#define MIN_ROUGHNESS 0.04

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_normal;
layout (location = 3) flat in uint in_primitive_index;



layout(binding = 2, set = 2, r32ui) volatile uniform uimage3D radiance_image;

layout(binding = 4, set = 0) uniform LightsInfo
{
    Light lights[MAX_LIGHTS];
}lights_info;

layout(push_constant) uniform PushConstant
{
    uint frame_index;
};


uint convVec4ToRGBA8(vec4 val) {
    return (uint(val.w) & 0x000000FF) << 24U |
    (uint(val.z) & 0x000000FF) << 16U |
    (uint(val.y) & 0x000000FF) << 8U |
    (uint(val.x) & 0x000000FF);
}


vec4 convRGBA8ToVec4(uint val)
{
    return vec4 (
    float(val & 0x000000ff),
    float((val & 0x0000ff00) >>  8u),
    float((val & 0x00ff0000) >> 16u),
    float((val & 0xff000000) >> 24u)
    );
}


void imageAtomicRGBA8Avg(ivec3 coords, vec4 value)
{
    imageStore(radiance_image, coords, uvec4(1));

    //value = vec4(1, 0, 0, 1);
    if (all(equal(value.xyz, vec3(0.001))))
    {
        return;
    }
    value.rgb *= 255.0;
    uint newVal = convVec4ToRGBA8(value);

    uint prevStoredVal = 0;
    uint curStoredVal;

    const int maxIterations = 255;
    int i = 0;
    vec4 curValF = vec4(0.0);
    while ((curStoredVal = imageAtomicCompSwap(radiance_image, coords, prevStoredVal, newVal)) != prevStoredVal && i < maxIterations)
    {
        i++;
        prevStoredVal = curStoredVal;
        vec4 rval = convRGBA8ToVec4(curStoredVal);
        rval.xyz = (rval.xyz * rval.w);// Denormalize
        curValF = rval + value;// Add new value
        curValF.xyz /= (curValF.w);// Renormalize
        newVal = convVec4ToRGBA8(curValF);
    }
    
    uint v = imageLoad(radiance_image, coords).x;
    vec4 curVal = convRGBA8ToVec4(v);
   // if (curVal.x <= 1e-3f && curVal.y <= 1e-3f && curVal.z <= 1e-3f)
//    {
//        debugPrintfEXT("curValF %f %f %f %f val %f %f %f %f iterations %d newval coord %d %d %d \n", curValF.x, curValF.y, curValF.z, curValF.w, value.x, value.y, value.z, value.w, i, coords.x, coords.y, coords.z);
//    }
    
//    if(curValF.x <= 1e-3f && curValF.y <= 1e-3f && curValF.z <= 1e-3f)
//    { 
//        debugPrintfEXT("curValF %f %f %f %f val %f %f %f %f iterations %d newval coord %d %d %d \n", curValF.x, curValF.y, curValF.z, curValF.w, value.x, value.y, value.z, value.w, i, coords.x, coords.y, coords.z);
//    }
    //    if (coords.x == 45 && coords.y == 77 && coords.z == 111)
    //    debugPrintfEXT("curValF %f %f %f %f val %f %f %f %f iterations %d newval coord %d %d %d \n", curValF.x, curValF.y, curValF.z, curValF.w, value.x, value.y, value.z, value.w, i, coords.x, coords.y, coords.z);
}

void voxelAtomicRGBA8Avg(ivec3 imageCoord, ivec3 faceIndex, vec4 color, vec3 weight)
{
    vec4 c = vec4(color.xyz * weight.x, 1.0);
    // debugPrintfEXT("color %f %f %f %f  weight %f %f %f \n", color.x, color.y, color.z, color.w, weight.x, weight.y, weight.z);
    imageAtomicRGBA8Avg(imageCoord + ivec3((clip_map_resoultion) * faceIndex.x, 0, 0), vec4(color.xyz * weight.x, 1.0));
    imageAtomicRGBA8Avg(imageCoord + ivec3((clip_map_resoultion) * faceIndex.y, 0, 0), vec4(color.xyz * weight.y, 1.0));
    imageAtomicRGBA8Avg(imageCoord + ivec3((clip_map_resoultion) * faceIndex.z, 0, 0), vec4(color.xyz * weight.z, 1.0));
    
    
}

void voxelAtomicRGBA8Avg6Faces(ivec3 imageCoord, vec4 color)
{
    for (uint i = 0; i < 6; ++i)
    {
        imageAtomicRGBA8Avg(imageCoord, color);
        imageCoord.x += int(clip_map_resoultion);
    }
}


ivec3 calculateVoxelFaceIndex(vec3 normal)
{

    return ivec3(
    normal.x > 0.0 ? 0 : 1,
    normal.y > 0.0 ? 2 : 3,
    normal.z > 0.0 ? 4 : 5
    );
}


void main(){

    vec3 world_pos = in_position;

    if (!pos_in_clipmap(world_pos)){
        discard;
    }

    ivec3 image_coords = computeImageCoords(world_pos);

    uint material_index = primitive_infos[in_primitive_index].material_index;
    GltfMaterial material = scene_materials[material_index];

    {


        vec3 diffuse_color            = vec3(0.0);
        vec3 specularColor            = vec3(0.0);
        vec4 base_color                = vec4(0.0, 0.0, 0.0, 1.0);
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

        base_color = material.pbrBaseColorFactor;
        if (material.pbrBaseColorTexture > -1)
        {
            base_color *= texture(scene_textures[material.pbrBaseColorTexture], in_uv);
        }

        // diffuse_color = base_color.rgb * (vec3(1.0) - f0) * (1.0 - metallic);
        diffuse_color = base_color.rgb;

        vec3 normal = normalize(in_normal);

        vec3 light_contribution = vec3(0.0);
        // Calculate light contribution here

        vec3 view_dir = normalize(per_frame.camera_pos - world_pos);

        PBRInfo pbr_info;
        // why use abs here?
        pbr_info.NdotV = clamp(abs(dot(normal, view_dir)), 0.001, 1.0);

        pbr_info.F0 = mix(vec3(0.04), diffuse_color, metallic);
        pbr_info.F90 = vec3(1.0);
        pbr_info.alphaRoughness = perceptual_roughness * perceptual_roughness;
        pbr_info.diffuseColor = diffuse_color;

        for (uint i = 0; i < per_frame.light_count; ++i)
        {
            Light light = lights_info.lights[i];
            vec3 light_dir = calcuate_light_dir(lights_info.lights[i], world_pos);
            vec3 half_vector = normalize(light_dir + view_dir);

            pbr_info.NdotL = clamp(dot(normal, light_dir), 0.001, 1.0);
            pbr_info.NdotH = clamp(dot(normal, half_vector), 0.0, 1.0);
            pbr_info.LdotH = clamp(dot(light_dir, half_vector), 0.0, 1.0);
            pbr_info.VdotH = clamp(dot(view_dir, half_vector), 0.0, 1.0);

            light_contribution +=
              apply_light(lights_info.lights[i], world_pos, normal) * microfacetBRDF(pbr_info) * calcute_shadow(lights_info.lights[i], world_pos);
        }
        
     //   light_contribution = vec3(1,0,0);
        if (all(equal(light_contribution.xyz, vec3(0.0))))
        {
            discard;
        }
        ivec3 faceIndex = calculateVoxelFaceIndex(-normal);
        voxelAtomicRGBA8Avg(image_coords, faceIndex, vec4(light_contribution, 1.0), vec3(1));
    }
}

