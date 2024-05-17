#ifndef COMMONS_DEVICE
#define COMMONS_DEVICE

#include "commons.h"
#include "PT/pt_commons.glsl"
#include "../common/sampling.glsl"

#include "util.glsl"


layout(binding = 0, set = 3) uniform accelerationStructureEXT tlas;

layout(binding = 2, set = 0) uniform SceneUboBuffer { SceneUbo scene_ubo; };
layout(binding = 3, set = 0) uniform SceneDescBuffer { SceneDesc scene_desc; };
layout(binding = 4, set = 0) readonly buffer Lights { RTLight lights[]; };
//todo fix this 
layout(binding = 0, set = 2, rgba32f) uniform image2D image;
layout(binding = 6, set = 0) uniform sampler2D scene_textures[];


layout(buffer_reference, std430, buffer_reference_align = 4) readonly buffer InstanceInfo{ RTPrimitive p[]; };
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



vec3 visualize_normal(vec3 n) {
    return (n+1.f)/2.f;
}





uint sample_distribution(float u, const uint64_t sample_distribution_addr, out float pdf) {
    Distribution1D distribution = Distribution1D(sample_distribution_addr);
    uint element_num = uint(distribution.m[0]);
    float func_int = distribution.m[1];

    //debugPrintfEXT("element_num func_int %d %f\n", element_num, func_int);

    uint left = 2;
    uint right = element_num + 2;
    while (left < right) {
        uint mid = (left + right) / 2;
        if (u < distribution.m[mid]) {
            right = mid;
        } else {
            left = mid + 1;
        }
    }
    uint idx = left - 2;
    pdf = distribution.m[idx+2 +element_num] / (func_int * element_num);
    return idx;
}

uint binary_search(float u, const float[5] cdf, uint cdf_begin, uint cdf_end) {
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

bool isBlack(vec3 v){
    return v.x == 0 && v.y == 0 && v.z == 0;
}




MeshSampleRecord uniform_sample_on_mesh(uint mesh_idx, vec3 rands, in mat4 world_matrix){

    MeshSampleRecord result;

    //first choose one triangle

    RTPrimitive mesh_info = prim_infos.p[mesh_idx];

    uint triangle_idx = sample_distribution(rands.x, mesh_info.area_distribution_buffer_addr, result.pdf);

    //    debugPrintfEXT("triangle_idx %d\n", triangle_idx);

    uint triangle_count = mesh_info.index_count / 3;

    //   triangle_idx = uint(rands.x * triangle_count);

    //    debugPrintfEXT("triangle_idx %d\n", triangle_idx);


    uint index_offset = mesh_info.index_offset + triangle_idx * 3;

    ivec3 ind = ivec3(indices.i[index_offset + 0], indices.i[index_offset + 1],
    indices.i[index_offset + 2]);

    uint vertex_offset = mesh_info.vertex_offset;

    ind += ivec3(vertex_offset);

    const vec3 p0 =  (world_matrix * vec4(vertices.v[ind.x], 1.f)).xyz;
    const vec3 p1 =  (world_matrix * vec4(vertices.v[ind.y], 1.f)).xyz;
    const vec3 p2 =  (world_matrix * vec4(vertices.v[ind.z], 1.f)).xyz;

    vec3 p0_ = vertices.v[ind.x];
    vec3 p1_ = vertices.v[ind.y];
    vec3 p2_ = vertices.v[ind.z];


    const vec4 n0 = vec4(normals.n[ind.x], 1.f);
    const vec4 n1 = vec4(normals.n[ind.y], 1.f);
    const vec4 n2 = vec4(normals.n[ind.z], 1.f);

    float u = 1 - sqrt(rands.y);
    float v = rands.z * sqrt(rands.y);
    const vec3 barycentrics = vec3(1.0 - u - v, u, v);

    mat4x4 inv_tr_mat = transpose(inverse(world_matrix));

    const vec4 nrm = normalize(n0 * barycentrics.x + n1 * barycentrics.y +
    n2 * barycentrics.z);

    result.pdf = 1.f / mesh_info.area;

    // float area = 0.5f * length(cross(p1 - p0, p2 - p0));
    // result.pdf  = 1.f / float(triangle_count) / area;

    result.n = normalize(vec3(inv_tr_mat * nrm));
    result.p = barycentrics.x * p0 + p1 * barycentrics.y + p2 * barycentrics.z;

    vec3 pos =  p0_ * barycentrics.x + p1_ * barycentrics.y + p2_ * barycentrics.z;
    result.p = (world_matrix * vec4(pos, 1.f)).xyz;



    result.triangle_idx = triangle_idx;

    return result;
}


//Param p: the point on the light source
//Param n_s: the normal of the light source
//Param w: the direction from the point to the light source
//Return: the radiance of the light source
vec3 eval_light(const RTLight light, const vec3 p, const vec3 n_g, const vec3 w){

    // return light.L;
    if (dot(n_g, -w) > 0){
        return light.L;
    }
    return vec3(0);
}


//return sample position,pdf,normal
//MeshSampleRecord uniform_sample_on_mesh(uint mesh_idx, vec3 rands, in mat4 world_matrix){
//
//    MeshSampleRecord result;
//
//    //first choose one triangle
//
//    RTPrimitive mesh_info = prim_infos.p[mesh_idx];
//
//    uint triangle_count = mesh_info.index_count / 3;
//
//    uint triangle_idx = uint(rands.x * triangle_count);
//
//    uint index_offset = mesh_info.index_offset + triangle_idx * 3;
//
//    ivec3 ind = ivec3(indices.i[index_offset + 0], indices.i[index_offset + 1],
//    indices.i[index_offset + 2]);
//
//    uint vertex_offset = mesh_info.vertex_offset;
//
//    ind += ivec3(vertex_offset);
//
//    const vec3 p0 = vertices.v[ind.x];
//    const vec3 p1 = vertices.v[ind.y];
//    const vec3 p2 = vertices.v[ind.z];
//
//    const vec3 n0 = normals.n[ind.x];
//    const vec3 n1 = normals.n[ind.y];
//    const vec3 n2 = normals.n[ind.z];
//
//    float u = 1 - sqrt(rands.y);
//    float v = rands.z * sqrt(rands.y);
//    const vec3 barycentrics = vec3(1.0 - u - v, u, v);
//
//    mat4x4 inv_tr_mat = transpose(inverse(world_matrix));
//
//    vec4 n = inv_tr_mat * vec4(normalize(n0 * barycentrics.x + n1 * barycentrics.y + n2 * barycentrics.z), 0.0);
//
//
//    float area = 0.5f * cross(p1 - p0, p2 - p0).length();
//
//    result.pdf  = 1.f / float(triangle_count) / area;
//    result.n =  normalize(vec3(inv_tr_mat * n));
//
//
//    vec3 pos =  p0 * barycentrics.x + p1 * barycentrics.y + p2 * barycentrics.z;
//    result.p = (world_matrix * vec4(pos, 1.f)).xyz;
//
//    //result.n = normalize(n.xyz);
//
//    result.triangle_idx = triangle_idx;
//
//    return result;
//}


float get_triangle_area(const uint prim_idx, const uint triangle_idx){

    RTPrimitive mesh_info = prim_infos.p[prim_idx];

    uint triangle_count = mesh_info.index_count / 3;
    mat4 world_matrix = mesh_info.world_matrix;

    uint index_offset = mesh_info.index_offset + triangle_idx * 3;

    ivec3 ind = ivec3(indices.i[index_offset + 0], indices.i[index_offset + 1],
    indices.i[index_offset + 2]);

    uint vertex_offset = mesh_info.vertex_offset;

    ind += ivec3(vertex_offset);

    vec3 p0 = vertices.v[ind.x];
    vec3 p1 = vertices.v[ind.y];
    vec3 p2 = vertices.v[ind.z];

    p0 = (world_matrix * vec4(p0, 1.f)).xyz;
    p1 = (world_matrix * vec4(p1, 1.f)).xyz;
    p2 = (world_matrix * vec4(p2, 1.f)).xyz;

    return 0.5f * cross(p1 - p0, p2 - p0).length();
}

float get_primitive_area(const uint prim_idx){

    RTPrimitive mesh_info = prim_infos.p[prim_idx];

    return mesh_info.area;
}


LightSample sample_li_area_light(const RTLight light, const SurfaceScatterEvent event, const vec3 rand){

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

    float cos_theta_light = dot(record.n, -wi);


    if (cos_theta_light <= 0.0){
        result.n = record.n;
        result.wi = wi;
        result.indensity  = vec3(0);
        result.pdf = 0;
        return result;
    }
    pdf = record.pdf * dist * dist  / abs(cos_theta_light);

    result.wi = wi;
    result.n = record.n;
    result.pdf = pdf;
    result.p = light_p;
    result.indensity = light.L;
    result.dist = dist;
    result.triangle_idx = record.triangle_idx;

    return result;
}

LightSample sample_li_infinite_light(const RTLight light, const SurfaceScatterEvent event, const vec3 rand){
    LightSample result;
    //    result.indensity = light.L;
    //    result.wi = -event.wo;
    //    result.pdf = 1.f;
    //    result.p = event.p;
    //    result.dist = 1000000.f;
    return result;
}

LightSample sample_li_point_light(const RTLight light, const SurfaceScatterEvent event, const vec3 rand){
    LightSample result;
    //    result.indensity = light.L;
    //    result.wi = normalize(light.position - event.p);
    //    result.pdf = 1.f;
    //    result.p = event.p;
    //    result.dist = length(light.position - event.p);
    return result;
}


LightSample sample_li(const RTLight light, const SurfaceScatterEvent event, const vec3 rand){
    uint light_type = light.light_type;
    if (light_type == RT_LIGHT_TYPE_AREA){
        return sample_li_area_light(light, event, rand);
    }
    if (light_type == RT_LIGHT_TYPE_INFINITE){
        return sample_li_infinite_light(light, event, rand);
    }
    if (light_type == RT_LIGHT_TYPE_POINT){
        return sample_li_point_light(light, event, rand);
    }
    LightSample result;
    return result;
}



#endif