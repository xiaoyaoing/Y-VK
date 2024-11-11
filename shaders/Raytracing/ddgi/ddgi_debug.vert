#version 460

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_debug_printf : enable

#include "uvsphere.glsl"
#include "ddgi_commons.h"

//layout (location = 0) in vec3 aPos;

layout (location = 0) out vec3 normal;
layout (location = 1) out flat int probe_index;

layout(set = 0, binding = 0) uniform _DDGIUboBuffer{ DDGIUbo ddgi_ubo; };
layout( set = 0,binding = 2) uniform _SceneUboBuffer { SceneUbo scene_ubo; };

#include  "ddgi_sample.glsl"



void main()
{
    probe_index = int(gl_InstanceIndex);

    vec3 position = UVSPHERE[gl_VertexIndex].xyz;
    ivec3 probe_grid =get_probe_coord_by_index(probe_index);
    vec3 probe_position = get_position_by_grid(probe_grid);

    normal = position;
    
    position *= 0.05f;
    position += probe_position;

//    debugPrintfEXT("probe_index: %d position: %f %f %f\n", probe_index, position.x, position.y, position.z);
    gl_Position = (scene_ubo.proj * scene_ubo.view) * vec4(position, 1.0);
   // gl_Position.y = 1- gl_Position.y;
  
}