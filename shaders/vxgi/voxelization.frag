#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_shader_atomic_float : require
#extension  GL_EXT_shader_non_constant_global_initializers : require

#extension GL_ARB_shader_image_load_store : require

precision mediump float;


#include "vxgi_common.h"
#include "../PerFrame.glsl"



layout(set = 0, binding = 2) uniform _VoxelizationParamater{
    VoxelizationParamater paramater;
} voxelization_paramater;

layout(binding = 0, set = 1, rgba32f) readonly  uniform sampler2D baseColorTexture;
layout(binding = 1, set = 1, rgba32f) writeonly uniform image3D opacity_image;
layout(binding = 2, set = 1, rgba32f) writeonly uniform image3D radiance_image;



uint clip_map_resoultion = voxelization_paramater.paramater.voxelResolution;
uint clipmap_level = voxelization_paramater.paramater.clipmapLevel;
float  voxel_size = voxelization_paramater.paramater.voxelSize;
float max_extent_world = voxelization_paramater.paramater.maxExtentWorld;
vec3 clipmap_min_pos = voxelization_paramater.paramater.clipmapMinPos;
vec3 clipmap_max_pos = voxelization_paramater.paramater.clipmapMaxPos;


ivec3 computeImageCoords(vec3 world_pos){


    vec3 clip_pos = fract(world_pos / max_extent_world);
    ivec3 imageCoords = ivec3(clip_pos * clip_map_resoultion) & ivec3(clip_map_resoultion - 1);

    //    #ifdef VOXEL_TEXTURE_WITH_BORDER
    //    imageCoords += ivec3(BORDER_WIDTH);
    //    #endif

    // Target the correct clipmap level
    imageCoords.y += int(clip_map_resoultion * clipmap_level);

    return imageCoords;
}

bool pos_in_clipmap(vec3 world_pos){
    return all(lessThan(world_pos, clipmap_max_pos)) && all(greaterThan(world_pos, clipmap_min_pos));
}

void main(){
    vec4 clip = (global_uniform.inv_view_proj * vec4(gl_FragCoord.xy, gl_FragDepth, 1.0));
    vec3 world_pos = (global_uniform.inv_view_proj * clip).xyz;

    if (!pos_in_clipmap(world_pos)){
        discard;
    }

    ivec3 image_coords = computeImageCoords(world_pos);
    for (int i = 0;i<6;i++){
        imageStore(opacity_image, image_coords + ivec3(0, 0, i), vec4(1.0));
        image_coords.x += int(clip_map_resoultion);
    }
}