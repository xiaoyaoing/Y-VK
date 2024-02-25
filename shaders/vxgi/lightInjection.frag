#version 460 core
#extension GL_GOOGLE_include_directive : enable

#extension GL_ARB_shader_image_load_store : require

precision mediump float;


// #include "vxgi_common.h"
#include "../PerFrame.glsl"
#include "../PerFrameShading.glsl"
#include "vxgiCommon.glsl"
#include "lighting.glsl"
#include "shadow.glsl"

layout( location = 0 ) in GS_OUT {
    vec3 position;
    vec2 texCoord;
    vec3 normal;
} fs_in;

layout(binding = 2, set = 2, rgba32f) writeonly uniform image3D radiance_image;

layout(binding = 4,set = 0 ) uniform LightsInfo
{
    Light lights[MAX_LIGHTS];
}lights_info;

layout ( push_constant ) uniform PushConstants
{
    uint uMaterialIndex;
};

void imageAtomicRGBA8Avg(ivec3 coords, vec4 value )
{
    value.rgb *= 255.0;
    uint newVal = convVec4ToRGBA8(value);
    uint prevStoredVal = 0; uint curStoredVal;

    // Loop as long as destination value gets changed by other threads
    while ( ( curStoredVal = imageAtomicCompSwap( uVoxelRadiance, coords, prevStoredVal, newVal )) != prevStoredVal)
    {
        prevStoredVal = curStoredVal;
        vec4 rval = convRGBA8ToVec4(curStoredVal);
        rval.xyz = (rval.xyz * rval.w); 	// Denormalize
        vec4 curValF = rval + value; 		// Add new value
        curValF.xyz /= (curValF.w); 		// Renormalize
        newVal = convVec4ToRGBA8(curValF);
    }
}

void voxelAtomicRGBA8Avg(ivec3 imageCoord, ivec3 faceIndex, vec4 color, vec3 weight)
{
    imageAtomicRGBA8Avg(imageCoord + ivec3((clip_map_resoultion ) * faceIndex.x, 0, 0), vec4(color.xyz * weight.x, 1.0));
    imageAtomicRGBA8Avg(imageCoord + ivec3((clip_map_resoultion ) * faceIndex.y, 0, 0), vec4(color.xyz * weight.y, 1.0));
    imageAtomicRGBA8Avg(imageCoord + ivec3((clip_map_resoultion ) * faceIndex.z, 0, 0), vec4(color.xyz * weight.z, 1.0));
}

void voxelAtomicRGBA8Avg6Faces(ivec3 imageCoord, vec4 color)
{
    for (uint i = 0; i < 6; ++i)
    {
        imageAtomicRGBA8Avg(imageCoord, color);
        imageCoord.x += clip_map_resoultion + 2;
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
    vec4 clip = (  vec4(gl_FragCoord.xy, gl_FragDepth, 1.0));
    vec3 world_pos = (per_frame.inv_view_proj * clip).xyz;

    if (!pos_in_clipmap(world_pos)){
        discard;
    }
    ivec3 image_coords = computeImageCoords(world_pos);

//    for (int i = 0;i<6;i++){
//        imageStore(radiance_image, image_coords + ivec3(0, 0, i), vec4(1.0));
//        image_coords.x += int(clip_map_resoultion);
//    }
    
    GltfMaterial material = scene_materials[uMaterialIndex];
    
    if(any(greaterThan(material.emissiveFactor, vec3(0.0)))){
        vec4 emission = vec4(material.emissiveFactor, 1.0);
        if (material.emissiveTexture > -1)
        {
            emission.rgb = texture(uTextures[material.emissiveTexture], fs_in.texCoord).rgb;
        }
        emission.rgb = clamp(emission.rgb, 0.0, 1.0);
        voxelAtomicRGBA8Avg6Faces(imageCoord, emission);    }
    else{

        for (uint i = 0U; i < per_frame.light_count; ++i)
        {
            L += apply_light(lights_info.lights[i], pos, normal);
        }


        vec4 color = material.pbrBaseColorFactor;
        if (material.pbrBaseColorTexture > -1)
        {
            color = texture(uTextures[material.pbrBaseColorTexture], fs_in.texCoord);
        }
        
        

        vec3 normal = normalize(fs_in.normal);
        vec3 lightDir = normalize(-uDirectionalLight.direction);

        vec3 lightContribution = vec3(0.0);
        // Calculate light contribution here
        
        for(uint i = 0; i < per_frame.light_count; ++i)
        {
            Light light = lights_info.lights[i];
            lightContribution += apply_light(light, fs_in.position, normal) * calcute_shadow(light,fs_in.position);
        }

        if (all(equal(lightContribution, vec3(0.0))))
        discard;

        vec3 radiance = lightContribution * color.rgb * color.a;
        radiance = clamp(radiance, 0.0, 1.0);
        ivec3 faceIndex = calculateVoxelFaceIndex(-normal);
        voxelAtomicRGBA8Avg(imageCoord, faceIndex, vec4(radiance, 1.0), abs(normal));
    }
}

