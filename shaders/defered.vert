#version  320 es
#extension GL_GOOGLE_include_directive : enable

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
#include "perFrame.glsl"

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord_0;


layout (location = 0) out vec2 o_uv;
layout (location = 1) out vec3 o_normal;

void main(void)
{
    vec4 pos = per_primitive.model * vec4(position, 1.0f);

    o_uv = texcoord_0;

    o_normal = mat3(per_primitive.model) * normal;

    gl_Position = per_frame.view_proj * pos;
    //gl_Position.z = 1.f / gl_Position.z;
}
