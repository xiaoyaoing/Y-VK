#version 460
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : require
#include "ddgi_commons.h"


layout(location = 0) in vec3 in_normal;
layout(location = 1) in flat uint in_probe_index;

layout(location = 0) out vec4 out_color;


layout(set = 0, binding = 0) uniform _DDGIUboBuffer{ DDGIUbo ddgi_ubo; };
layout(binding = 2, set = 0) uniform _SceneUboBuffer { SceneUbo scene_ubo; };
layout(set = 1, binding = 0) uniform sampler2D radiance_map;

#include  "ddgi_sample.glsl"


void main(){
    uvec3 probeCoord = get_probe_coord_by_index(in_probe_index);
    vec2 uv = get_probe_color_uv(probeCoord,in_normal);
    vec3 radiance = texture(radiance_map, uv).rgb;
    out_color = vec4(radiance, 1.0);
}