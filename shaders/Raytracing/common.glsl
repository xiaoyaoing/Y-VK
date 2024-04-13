#ifndef COMMONS_DEVICE
#define COMMONS_DEVICE


#include "commons.h"
#include "PT/pt_commons.glsl"
#include "util.glsl"


layout(binding = 0, set = 3) uniform accelerationStructureEXT tlas;

layout(binding = 2, set = 0) uniform SceneUboBuffer { SceneUbo scene_ubo; };
layout(binding = 3, set = 0) uniform SceneDescBuffer { SceneDesc scene_desc; };
layout(binding = 4, set = 0) readonly buffer Lights { RTLight lights[]; };
//todo fix this 
layout(binding = 0, set = 2, rgba32f) uniform image2D image;
layout(binding = 0, set = 1) uniform sampler2D scene_textures[];


layout(buffer_reference, scalar, buffer_reference_align = 4) readonly buffer InstanceInfo{ RTPrimitive p[]; };
layout(buffer_reference, scalar, buffer_reference_align = 4) readonly buffer Vertices { vec3 v[]; };
layout(buffer_reference, scalar, buffer_reference_align = 4) readonly buffer Indices { uint i[]; };
layout(buffer_reference, scalar, buffer_reference_align = 4) readonly buffer Normals { vec3 n[]; };
layout(buffer_reference, scalar, buffer_reference_align = 4) readonly buffer TexCoords { vec2 t[]; };
layout(buffer_reference, scalar, buffer_reference_align = 4) readonly buffer Materials { RTMaterial m[]; };
layout(buffer_reference, scalar, buffer_reference_align = 4) readonly buffer Distribution1D { float m[]; };


Indices indices = Indices(scene_desc.index_addr);
Vertices vertices = Vertices(scene_desc.vertex_addr);
Normals normals = Normals(scene_desc.normal_addr);
Materials materials = Materials(scene_desc.material_addr);
InstanceInfo prim_infos = InstanceInfo(scene_desc.prim_info_addr);
TexCoords tex_coords = TexCoords(scene_desc.uv_addr);



struct LightSample{
    vec3 indensity;
    vec3 wi;
    float pdf;

    uint triangle_idx;

    vec3 p;
    vec3 n;
    float dist;
};

struct MeshSampleRecord{
    vec3 n;
    vec3 p;
    float pdf;
    uint triangle_idx;
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

float rand1(inout uvec4 rng_state) {
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

uint binary_search(float u, const float[] cdf,uint cdf_begin, uint cdf_end) {
    uint left = cdf_begin;
    uint right = cdf_end;
    while (left < right) {
        uint mid = (left + right) / 2;
        if (u < cdf[mid]) {
            right = mid;
        } else {
            left = mid + 1;
        }
    }
    return left;
}

uint sample_distribution(float u,  const Distribution1D distribution,out pdf) {
    uint element_num = uint(distribution.m[0]);
    float func_int = distribution.m[1];
    uint idx = binary_search(u, distribution.m, 2, element_num + 2);
    pdf = distribution.m[idx+2 +element_num] / (func_int * element_num);
    return idx;
}


MeshSampleRecord uniform_sample_on_mesh(uint mesh_idx, vec3 rands, in mat4 world_matrix, uint triangle_idx){

    MeshSampleRecord result;
    
    //first choose one triangle

    RTPrimitive mesh_info = prim_infos.p[mesh_idx];

    Distribution1D dist = Distribution1D(area_distribution_buffer_addr);
    triangle_idx = sample_distribution(rands.x, dist, result.pdf);

    uint triangle_count = mesh_info.index_count / 3;

    uint index_offset = mesh_info.index_offset + triangle_idx * 3;

    ivec3 ind = ivec3(indices.i[index_offset + 0], indices.i[index_offset + 1],
    indices.i[index_offset + 2]);

    uint vertex_offset = mesh_info.vertex_offset;

    ind += ivec3(vertex_offset);

    const vec3 p0 = vertices.v[ind.x];
    const vec3 p1 = vertices.v[ind.y];
    const vec3 p2 = vertices.v[ind.z];

    const vec3 n0 = normals.n[ind.x];
    const vec3 n1 = normals.n[ind.y];
    const vec3 n2 = normals.n[ind.z];

    float u = 1 - sqrt(rands.y);
    float v = rands.z * sqrt(rands.y);
    const vec3 barycentrics = vec3(1.0 - u - v, u, v);

    mat4x4 inv_tr_mat = transpose(inverse(world_matrix));

    vec4 n = inv_tr_mat * vec4(normalize(n0 * barycentrics.x + n1 * barycentrics.y + n2 * barycentrics.z), 0.0);

    float area = 0.5f * cross(p1 - p0, p2 - p0).length();

    result.pdf /= area;
    result.n =  normalize(vec3(inv_tr_mat * n));

    //  result.n = normalize(n0 * barycentrics.x + n1 * barycentrics.y + n2 * barycentrics.z);
    // result.n = vec3(mesh_idx / 10.f);
    //    result.n = vec3(normalize(n0));

    vec3 pos =  p0 * barycentrics.x + p1 * barycentrics.y + p2 * barycentrics.z;
    result.p = (world_matrix * vec4(pos, 1.f)).xyz;

    //result.n = normalize(n.xyz);

    result.triangle_idx = triangle_idx;

    return result;
}


//return sample position,pdf,normal
MeshSampleRecord uniform_sample_on_mesh(uint mesh_idx, vec3 rands, in mat4 world_matrix){

    MeshSampleRecord result;

    //first choose one triangle

    RTPrimitive mesh_info = prim_infos.p[mesh_idx];

    uint triangle_count = mesh_info.index_count / 3;

    uint triangle_idx = uint(rands.x * triangle_count);

    uint index_offset = mesh_info.index_offset + triangle_idx * 3;

    ivec3 ind = ivec3(indices.i[index_offset + 0], indices.i[index_offset + 1],
    indices.i[index_offset + 2]);

    uint vertex_offset = mesh_info.vertex_offset;

    ind += ivec3(vertex_offset);

    const vec3 p0 = vertices.v[ind.x];
    const vec3 p1 = vertices.v[ind.y];
    const vec3 p2 = vertices.v[ind.z];

    const vec3 n0 = normals.n[ind.x];
    const vec3 n1 = normals.n[ind.y];
    const vec3 n2 = normals.n[ind.z];

    float u = 1 - sqrt(rands.y);
    float v = rands.z * sqrt(rands.y);
    const vec3 barycentrics = vec3(1.0 - u - v, u, v);

    mat4x4 inv_tr_mat = transpose(inverse(world_matrix));

    vec4 n = inv_tr_mat * vec4(normalize(n0 * barycentrics.x + n1 * barycentrics.y + n2 * barycentrics.z), 0.0);


    float area = 0.5f * cross(p1 - p0, p2 - p0).length();

    result.pdf  = 1.f / float(triangle_count) / area;
    result.n =  normalize(vec3(inv_tr_mat * n));


    vec3 pos =  p0 * barycentrics.x + p1 * barycentrics.y + p2 * barycentrics.z;
    result.p = (world_matrix * vec4(pos, 1.f)).xyz;

    //result.n = normalize(n.xyz);

    result.triangle_idx = triangle_idx;

    return result;
}


float get_triangle_area(const uint prim_idx, const uint triangle_idx){

    RTPrimitive mesh_info = prim_infos.p[prim_idx];

    uint triangle_count = mesh_info.index_count / 3;

    uint index_offset = mesh_info.index_offset + triangle_idx * 3;

    ivec3 ind = ivec3(indices.i[index_offset + 0], indices.i[index_offset + 1],
    indices.i[index_offset + 2]);

    uint vertex_offset = mesh_info.vertex_offset;

    ind += ivec3(vertex_offset);

    const vec3 p0 = vertices.v[ind.x];
    const vec3 p1 = vertices.v[ind.y];
    const vec3 p2 = vertices.v[ind.z];

    return 0.5f * cross(p1 - p0, p2 - p0).length();
}


LightSample sample_li(const RTLight light, const SurfaceScatterEvent event, const vec3 rand){

    LightSample result;


    //  result.indensity = light_idx == 0 ? vec3(1.f) : vec3(0.f);
    result.indensity = light.L;
    //return result;

    float pdf;
    //sample one point on primitive 
    MeshSampleRecord record  = uniform_sample_on_mesh(light.prim_idx, rand, light.world_matrix);

    //    record.n = -record.n;

    vec3 light_p = record.p;
    vec3 p = event.p;

    vec3 wi = light_p - p;
    float dist = length(wi);

    wi /= dist;

    dist -=  EPS;

    //    result.indensity = abs( record.n);
    //    return result;

    float cos_theta_light = dot(record.n, -wi);
    if (cos_theta_light <= 0.0){
        result.n = record.n;
        result.indensity  = vec3(0);
        result.pdf = 0;
        return result;
    }
    //convert pdf
    pdf = record.pdf * dist * dist / abs(cos_theta_light);

    result.wi = wi;
    result.n = record.n;
    result.pdf = pdf;
    result.p = light_p;
    result.indensity = light.L;
    result.dist = dist;
    result.triangle_idx = record.triangle_idx;

    return result;
}




#endif