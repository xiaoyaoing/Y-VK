#version 450
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_debug_printf : enable
#extension GL_ARB_shader_image_load_store : require

precision mediump float;


// #include "vxgi_common.h"
#include "vxgi.glsl"
#include "../PerFrame.glsl"

layout(location = 0) in vec3 in_world_pos;



layout(binding = 1, set = 2, rgba8) writeonly uniform image3D opacity_image;


void main(){
    //image_coords: 8 261 118 world_pos: 4.093658 2.595914 -4.950000 2
    vec3 world_pos = in_world_pos;

    if (!pos_in_clipmap(world_pos)){
        discard;
    }
    ivec3 image_coords = computeImageCoords(world_pos);

    // debugPrintfEXT("image_coords: %d %d %d world_pos: %f %f %f %d\n", image_coords.x, image_coords.y, image_coords.z, world_pos.x, world_pos.y, world_pos.z, clipmap_level);

    for (int i = 0;i<6;i++){
        imageStore(opacity_image, image_coords, vec4(1.0));
        image_coords.x += int(clip_map_resoultion);
    }
}