#version 450
/* Copyright (c) 2019-2020, Arm Limited and Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 the "License";
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
precision highp float;


struct Light
{
    vec4 color;// color.w represents light intensity
    vec4 position;// position.w represents type of light
    vec4 direction;// direction.w represents range
    vec2 info;// (only used for spot lights) info.x represents light inner cone angle, info.y represents light outer cone angle
};

vec3 apply_directional_light(Light light, vec3 normal)
{
    vec3 world_to_light = -light.direction.xyz;
    world_to_light      = normalize(world_to_light);\
    float ndotl         = clamp(dot(normal, world_to_light), 0.0, 1.0);
    return ndotl * light.color.w * light.color.rgb;
}

vec3 apply_point_light(Light light, vec3 pos, vec3 normal)
{
    vec3  world_to_light = light.position.xyz - pos;
    float dist           = length(world_to_light) * 0.005f;
    float atten          = 1.0 / (dist * dist);
    world_to_light       = normalize(world_to_light);
    float ndotl          = clamp((dot(normal, world_to_light)), 0.0, 1.0);
    return ndotl * light.color.w * atten * light.color.rgb;
}

layout(input_attachment_index = 0, binding = 0) uniform subpassInput i_albedo;
layout(input_attachment_index = 1, binding = 1) uniform subpassInput i_position;
layout(input_attachment_index = 2, binding = 2) uniform subpassInput i_normal;

layout(set = 0, binding = 3) uniform GlobalUniform {
    vec3 viewPos;
    vec3 lightPos;
} poses;

layout(location = 0) in vec2 in_uv;
layout(location = 0) out vec4 o_color;

// layout(set = 0, binding = 3) uniform GlobalUniform
// {
//     mat4 inv_view_proj;
//     vec2 inv_resolution;
// }
// global_uniform;

//#include "lighting.h"

layout(set = 0, binding = 4) uniform LightsInfo
{
    Light directional_lights[32];
    Light point_lights[32];
    Light spot_lights[32];
}
lights_info;

// layout(constant_id = 0) const uint DIRECTIONAL_LIGHT_COUNT = 0U;
// layout(constant_id = 1) const uint POINT_LIGHT_COUNT       = 0U;
// layout(constant_id = 2) const uint SPOT_LIGHT_COUNT        = 0U;

void main()
{
    //Retrieve position from depth
    // vec4  clip         = vec4(in_uv * 2.0 - 1.0, subpassLoad(i_depth).x, 1.0);
    // highp vec4 world_w = global_uniform.inv_view_proj * clip;
    // highp vec3 pos     = world_w.xyz / world_w.w;
    vec4 albedo = subpassLoad(i_albedo);
    vec4 pos = subpassLoad(i_position) * 20000.f - 10000.f;
    // Transform from [0,1] to [-1,1]
    vec3 normal = subpassLoad(i_normal).xyz;
    normal      = normalize(2.0 * normal - 1.0);

    vec3 L = vec3(0.0);

    // Calculate lighting
    //     for (uint i = 0U; i < DIRECTIONAL_LIGHT_COUNT; ++i)
    //     {
    //     	L += apply_directional_light(lights_info.directional_lights[i], normal);
    //     }
    for (uint i = 0U; i < 32; ++i)
    {
        L += apply_point_light(lights_info.point_lights[i], pos.xyz, normal);
    }
    //     for (uint i = 0U; i < SPOT_LIGHT_COUNT; ++i)
    //     {
    //     	L += apply_spot_light(lights_info.spot_lights[i], pos, normal);
    //     }

    vec3 ambient_color = vec3(0.2) * albedo.xyz;

    o_color = vec4(ambient_color + L * albedo.xyz, albedo.a);

    //    vec3 L = normalize(poses.lightPos-pos.xyz);
    //    vec3 V = normalize(poses.viewPos-pos.xyz);
    //    vec3 H = normalize(L+V);
    //
    //    vec3 diffuse = max(dot(normal, L), 0.1f).rrr;
    //    float specular = pow(max(dot(H, normal), 0.f), 32);
    //
    //    o_color = vec4(albedo.xyz * diffuse + specular, albedo.a);
}