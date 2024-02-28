#version 450
#extension GL_GOOGLE_include_directive : enable

#extension GL_ARB_shader_image_load_store : require

precision mediump float;


// #include "vxgi_common.h"
#include "vxgi.glsl"
#include "../PerFrame.glsl"



layout(binding = 1, set = 2, rgba32f) writeonly uniform image3D opacity_image;


void main(){
    vec4 clip = (vec4(gl_FragCoord.xy, gl_FragDepth, 1.0));
    vec3 world_pos = (per_frame.inv_view_proj * clip).xyz;

    if (!pos_in_clipmap(world_pos)){
        discard;
    }
    ivec3 image_coords = computeImageCoords(world_pos);

    for (int i = 0;i<6;i++){
        imageStore(opacity_image, image_coords + ivec3(0, 0, i), vec4(1.0));
        image_coords.x += int(clip_map_resoultion);
    }
}