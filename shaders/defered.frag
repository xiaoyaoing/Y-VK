#version 460 core
#extension GL_GOOGLE_include_directive : enable
//#extension GL_EXT_nonuniform_qualifier : enable
/* Copyright (c) 2019, Arm Limited and Contributors
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

#include "perFrameShading.glsl"
#include "perFrame.glsl"


// #ifdef HAS_baseColorTexture
//layout (set=1, binding=0) uniform sampler2D baseColorTexture;
// #endif

layout (location = 0) in vec2 in_uv;
layout (location = 1) in vec3 in_normal;
layout (location = 2) flat in uint in_primitive_index;

layout (location = 0) out vec4 o_albedo;
layout (location = 1) out vec4 o_normal;

layout(std430, set = 0, binding = 2) readonly buffer _GlobalPrimitiveUniform {
    PerPrimitive primitive_infos[];
};

//layout(push_constant) uniform PushConstants {
//    uint material_index;
//};

void main(void)
{
    vec3 normal = normalize(in_normal);
    // Transform normals from [-1, 1] to [0, 1]
    o_normal = vec4(0.5 * normal + 0.5, 1.0);

    uint material_index = primitive_infos[in_primitive_index].material_index;
    GltfMaterial material = scene_materials[material_index];
    vec4 base_color = vec4(0);


    if (material.pbrBaseColorTexture != -1)
    {
        base_color = texture(scene_textures[material.pbrBaseColorTexture], in_uv);
    }
    else
    {
        base_color = material.pbrBaseColorFactor;
    }

    o_albedo = base_color;
    //    o_normal =  o_albedo;


}
