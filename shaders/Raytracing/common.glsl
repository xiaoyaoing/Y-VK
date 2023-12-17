#ifndef COMMONS_DEVICE
#define COMMONS_DEVICE


#include "commons.h"
#include "PT/pt_commons.glsl"
#include "util.glsl"


layout(binding = 0, set = 0) uniform accelerationStructureEXT tlas;
layout(binding = 1, set = 0, rgba32f) uniform image2D image;
layout(binding = 2, set = 0) uniform SceneUboBuffer {SceneUbo scene_ubo;};
layout(binding = 3, set = 0) uniform SceneDescBuffer {SceneDesc scene_desc;};
layout(binding = 4, set = 0) readonly buffer Lights {RTLight lights[];};
layout(binding = 5, set = 0) uniform sampler2D scene_textures[];


layout(buffer_reference, scalar, buffer_reference_align = 4) readonly buffer InstanceInfo{ RTPrimitive p[];} ;
layout(buffer_reference, scalar, buffer_reference_align = 4) readonly buffer Vertices { vec3 v[]; };
layout(buffer_reference, scalar, buffer_reference_align = 4) readonly buffer Indices { uint i[]; };
layout(buffer_reference, scalar, buffer_reference_align = 4) readonly buffer Normals { vec3 n[]; };
layout(buffer_reference, scalar, buffer_reference_align = 4) readonly buffer TexCoords { vec2 t[]; };
layout(buffer_reference, scalar, buffer_reference_align = 4) readonly buffer Materials { RTMaterial m[]; };


Indices indices = Indices(scene_desc.index_addr);
Vertices vertices = Vertices(scene_desc.vertex_addr);
Normals normals = Normals(scene_desc.normal_addr);
Materials materials = Materials(scene_desc.material_addr);
InstanceInfo prim_infos = InstanceInfo(scene_desc.prim_info_addr);
TexCoords tex_coords = TexCoords(scene_desc.uv_addr);

vec3 eval_bsdf(uint material_idx,vec3 n,vec3 wi,vec3 wo){
    return vec3(1.0f);
}

struct LightSample{
    vec3 indensity;
    vec3 wi;
    float pdf;
    vec3 p;
};

struct MeshSampleRecord{
    vec3 n;
    vec3 p;
    float pdf;
};


uvec4 init_rng(uvec2 pixel_coords, uvec2 resolution, uint frame_num) {
    return uvec4(pixel_coords.xy, frame_num, 0);
}

float uint_to_float(uint x) {
    return uintBitsToFloat(0x3f800000 | (x >> 9)) - 1.0f;
}

uvec4 pcg4d(uvec4 v) {
    v = v * 1664525u + 1013904223u;
    v.x += v.y * v.w;
    v.y += v.z * v.x;
    v.z += v.x * v.y;
    v.w += v.y * v.z;
    v = v ^ (v >> 16u);
    v.x += v.y * v.w;
    v.y += v.z * v.x;
    v.z += v.x * v.y;
    v.w += v.y * v.z;
    return v;
}

float rand(inout uvec4 rng_state) {
    rng_state.w++;
    return uint_to_float(pcg4d(rng_state).x);
}

vec2 rand2(inout uvec4 rng_state) {
    rng_state.w++;
    uvec4 pcg = pcg4d(rng_state);
    return vec2(uint_to_float(pcg.x), uint_to_float(pcg.y));
}

vec3 rand3(inout uvec4 rng_state) {
    rng_state.w++;
    uvec4 pcg = pcg4d(rng_state);
    return vec3(uint_to_float(pcg.x), uint_to_float(pcg.y), uint_to_float(pcg.z));
}

vec4 rand4(inout uvec4 rng_state) {
    rng_state.w++;
    uvec4 pcg = pcg4d(rng_state);
    return vec4(uint_to_float(pcg.x), uint_to_float(pcg.y), uint_to_float(pcg.z), uint_to_float(pcg.w));
}




//return sample position,pdf,normal
MeshSampleRecord uniform_sample_on_mesh(uint mesh_idx,vec3 rands,in mat4 world_matrix){
    
    MeshSampleRecord result;
    
    //first choose one triangle
    
    RTPrimitive mesh_info = prim_infos.p[mesh_idx];

    uint triangle_count = mesh_info.index_count / 3;

    uint triangle_idx = uint(rands.x * triangle_count);
    
    uint index_offset = mesh_info.index_offset + triangle_idx * 3;
    
    vec3 p0 = vertices.v[indices.i[index_offset]];
    vec3 p1 = vertices.v[indices.i[index_offset + 1]];
    vec3 p2 = vertices.v[indices.i[index_offset + 2]];
    
    vec3 n0 = normals.n[indices.i[index_offset]];
    vec3 n1 = normals.n[indices.i[index_offset + 1]];
    vec3 n2 = normals.n[indices.i[index_offset + 2]];
    
    float u = 1 - sqrt(rands.y);
    float v = rands.z * sqrt(rands.y);
    const vec3 barycentrics = vec3(1.0 - u - v, u, v);
    
    mat4x4 inv_tr_mat = transpose(inverse(world_matrix));
    
    vec4 n = inv_tr_mat * vec4(normalize(n0 * barycentrics.x + n1 * barycentrics.y + n2 * barycentrics.z), 0.0);
    
    result.pdf  = 2.f / float(triangle_count) / dot(vec3(n), normalize(p1 - p0));
    result.n = vec3(n);
    result.p = p0 * barycentrics.x + p1 * barycentrics.y + p2 * barycentrics.z;
    
    return result;
}

LightSample sample_li(uint light_idx,vec3 p,vec3 n,vec3 wo,vec3 rand){

    LightSample result;

    RTLight light = lights[light_idx];
    

    vec3 light_ng;
    float pdf;
    //sample one point on primitive 
    MeshSampleRecord record  = uniform_sample_on_mesh(light.prim_idx, rand, light.world_matrix);


    vec3 light_p = record.p; 
    
    vec3 wi = light_p - p;
    float dist = length(wi) - EPS;

//    any_hit_payload.hit = 1;
//    traceRayEXT(tlas,
//    gl_RayFlagsTerminateOnFirstHitEXT |
//    gl_RayFlagsSkipClosestHitShaderEXT, 0xFF, 1, 0, 1, p, 0, wi, dist - EPS, 1);

//    bool visible = any_hit_payload.hit == 0;
//    if (!visible){
//        return result;
//    }
    float cos_theta_light = dot(record.n, -wi);
    if (cos_theta_light <= 0.0){
        return result;
    }

    //convert pdf
    pdf = record.pdf *  dist * dist / abs(dot(light_ng, wi));

    result.pdf = pdf;
    // result.wi = wi;
    result.indensity = light.L * abs(dot(record.n, wi));

    return result;
}

LightSample uniform_sample_one_light(vec4 u,vec3 p,vec3 wo,vec3 n,uint light_num){
    uint light_idx = uint(u.x * light_num);
    float light_choose_pdf = 1.0 / light_num;
    LightSample result = sample_li(light_idx,p,n,wo,u.yzw);
   // result.indensity = vec3(1.f);
    result.pdf *= light_choose_pdf;
    return result;
}

#endif