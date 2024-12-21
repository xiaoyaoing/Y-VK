#version 460
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_debug_printf : enable
#include "ddgi_commons.h"


layout(location = 0) in vec3 in_normal;
layout(location = 1) in flat int in_probe_index;

layout(location = 0) out vec4 out_color;


layout(set = 0, binding = 0) uniform _DDGIUboBuffer{ DDGIUbo ddgi_ubo; };
layout(binding = 2, set = 0) uniform _SceneUboBuffer { SceneUbo scene_ubo; };
layout(set = 1, binding = 0) uniform sampler2D radiance_map;

#include  "ddgi_sample.glsl"


void main(){
    ivec3 probeCoord = get_probe_coord_by_index(in_probe_index);
    vec2 uv = get_probe_color_uv(probeCoord,vec3(0,0,1),PROBE_RADIANCE_SIDE);
    
    vec3 normal = in_normal;
    if(normal.y > 0){
        if(normal.z<0){
          //  normal.x = -normal.x;
        }
       // normal.x = -normal.x;
//        normal.z = -normal.z;
    }
    
   // if(normal.z >= 1e-5f)discard;
    
    if(normal.y < 0){
      //  nor
        //normal = -normal;
    }
    
    uv = get_probe_color_uv(probeCoord,normal,PROBE_RADIANCE_SIDE);
    vec3 radiance = texture(radiance_map, uv).rgb;
    if(radiance.z > 0.8){
      //  radiance = normal;
    }
    if(normal == vec3(0,1,0)){
        
    }

    vec2 pixel_coord = 0.5f * vec2(PROBE_RADIANCE_SIDE+2);
    pixel_coord += (encode_to_oct(normal)) * 0.5 * float(PROBE_RADIANCE_SIDE);
    
    
    if(int(pixel_coord.x) == 4 && int(pixel_coord.y) == 4){
      //  debugPrintfEXT("normal: %f %f %f pixel_coord: %f %f\n",normal.x,normal.y,normal.z,pixel_coord.x,pixel_coord.y);
    }
    
    bool is_border = pixel_coord.x == 0 || pixel_coord.y == 0 || pixel_coord.x == PROBE_RADIANCE_SIDE || pixel_coord.y == PROBE_RADIANCE_SIDE;
    if(is_border){
        radiance = vec3(1,0,0);
    }
    
//    radiance.x = int(pixel_coord.x) / float(PROBE_RADIANCE_SIDE);
//    radiance.y = int(pixel_coord.y) / float(PROBE_RADIANCE_SIDE);
   // radiance = normalize(radiance) - normalize(normal);
   // debugPrintfEXT("radiance: %f %f %f\n",radiance.x,radiance.y,radiance.z);
    out_color = vec4(radiance, 1.0);
}