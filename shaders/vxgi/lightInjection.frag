#version 450
#extension GL_GOOGLE_include_directive : require
//#extension GLSL_EXT_shader_image_int64 : require

#extension GL_ARB_shader_image_load_store : require

precision mediump float;


// #include "vxgi_common.h"
#include "../PerFrame.glsl"
#include "../PerFrameShading.glsl"
#include "vxgi.glsl"
#include "../shadow.glsl"

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

layout(binding = 2, set = 2, r32ui) volatile uniform uimage3D radiance_image;

layout(binding = 4, set = 0) uniform LightsInfo
{
    Light lights[MAX_LIGHTS];
}lights_info;

layout (push_constant) uniform PushConstants
{
    uint uMaterialIndex;
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
    // imageStore(radiance_image, coords, value);
    value.rgb *= 255.0;
    uint newVal = convVec4ToRGBA8(value);
    uint prevStoredVal = 0;
    uint curStoredVal;

    // Loop as long as destination value gets changed by other threads
    while ((curStoredVal = imageAtomicCompSwap(radiance_image, coords, prevStoredVal, newVal)) != prevStoredVal)
    {
        prevStoredVal = curStoredVal;
        vec4 rval = convRGBA8ToVec4(curStoredVal);
        rval.xyz = (rval.xyz * rval.w);// Denormalize
        vec4 curValF = rval + value;// Add new value
        curValF.xyz /= (curValF.w);// Renormalize
        newVal = convVec4ToRGBA8(curValF);
    }
}

void voxelAtomicRGBA8Avg(ivec3 imageCoord, ivec3 faceIndex, vec4 color, vec3 weight)
{
    imageAtomicRGBA8Avg(imageCoord + ivec3((clip_map_resoultion) * faceIndex.x, 0, 0), vec4(color.xyz * weight.x, 1.0));
    imageAtomicRGBA8Avg(imageCoord + ivec3((clip_map_resoultion) * faceIndex.y, 0, 0), vec4(color.xyz * weight.y, 1.0));
    imageAtomicRGBA8Avg(imageCoord + ivec3((clip_map_resoultion) * faceIndex.z, 0, 0), vec4(color.xyz * weight.z, 1.0));
}

void voxelAtomicRGBA8Avg6Faces(ivec3 imageCoord, vec4 color)
{
    for (uint i = 0; i < 6; ++i)
    {
        imageAtomicRGBA8Avg(imageCoord, color);
        imageCoord.x += int(clip_map_resoultion) + 2;
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

    if (!pos_in_clipmap(position)){
        discard;
    }
    ivec3 image_coords = computeImageCoords(position);

    //    for (int i = 0;i<6;i++){
    //        imageStore(radiance_image, image_coords + ivec3(0, 0, i), vec4(1.0));
    //        image_coords.x += int(clip_map_resoultion);
    //    }

    GltfMaterial material = scene_materials[uMaterialIndex];

    if (any(greaterThan(material.emissiveFactor, vec3(0.0)))){
        vec4 emission = vec4(material.emissiveFactor, 1.0);
        if (material.emissiveTexture > -1)
        {
            emission.rgb = texture(scene_textures[material.emissiveTexture], texCoord).rgb;
        }
        emission.rgb = clamp(emission.rgb, 0.0, 1.0);
        voxelAtomicRGBA8Avg6Faces(image_coords, emission); }
    else {


        vec4 color = material.pbrBaseColorFactor;
        if (material.pbrBaseColorTexture > -1)
        {
            color = texture(scene_textures[material.pbrBaseColorTexture], texCoord);
        }

        vec3 normal = normalize(normal);

        vec3 lightContribution = vec3(0.0);
        // Calculate light contribution here

        for (uint i = 0; i < per_frame.light_count; ++i)
        {
            Light light = lights_info.lights[i];
            lightContribution += apply_light(light, position, normal) * calcute_shadow(light, position);
        }

        if (all(equal(lightContribution, vec3(0.0))))
        discard;

        vec3 radiance = lightContribution * color.rgb * color.a;
        radiance = clamp(radiance, 0.0, 1.0);
        ivec3 faceIndex = calculateVoxelFaceIndex(-normal);
        voxelAtomicRGBA8Avg(image_coords, faceIndex, vec4(radiance, 1.0), abs(normal));
    }
}

