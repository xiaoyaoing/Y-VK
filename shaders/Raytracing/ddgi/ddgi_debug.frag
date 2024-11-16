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
    vec2 uv = get_probe_color_uv(probeCoord,in_normal,PROBE_RADIANCE_SIDE);
    vec3 radiance = texture(radiance_map, uv).rgb;
    
//    debugPrintfEXT("Radiance: %f %f %f\n", radiance.x, radiance.y, radiance.z);
//    radiance = in_normal;
  //  radiance = vec3(uv,0);
    out_color = vec4(radiance, 1.0);
}