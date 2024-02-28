#version 460 core
#extension GL_GOOGLE_include_directive : enable

#include "perFrame.glsl"
#include "lighting.glsl"

precision mediump float;




layout(input_attachment_index = 0, binding = 0, set=2) uniform subpassInput i_albedo;
layout(input_attachment_index = 1, binding = 1, set=2) uniform subpassInput i_depth;
layout(input_attachment_index = 2, binding = 2, set=2) uniform subpassInput i_normal;

layout(location = 0) in vec2 in_uv;
layout(location = 0) out vec4 o_color;

layout(binding = 4) uniform LightsInfo
{
    Light lights[MAX_LIGHTS];
}lights_info;

// layout(constant_id = 0) const uint DIRECTIONAL_LIGHT_COUNT = 0U;
// layout(constant_id = 1) const uint POINT_LIGHT_COUNT       = 0U;
// layout(constant_id = 2) const uint SPOT_LIGHT_COUNT        = 0U;

void main()
{
    //Retrieve position from depth
    vec4  clip         = vec4(in_uv * 2.0 - 1.0, subpassLoad(i_depth).x, 1.0);
    vec4 world_w = per_frame.inv_view_proj * clip;
    vec3 pos     = world_w.xyz / world_w.w;

    vec4 albedo = subpassLoad(i_albedo);
    vec3 normal = subpassLoad(i_normal).xyz;
    normal      = normalize(2.0 * normal - 1.0);

    vec3 L = vec3(0.0);

    for (uint i = 0U; i < per_frame.light_count; ++i)
    {
        L += apply_light(lights_info.lights[i], pos, normal);
    }


    vec3 ambient_color = vec3(0.2) * albedo.xyz;

    o_color = vec4(L * albedo.xyz + ambient_color, albedo.a);
}