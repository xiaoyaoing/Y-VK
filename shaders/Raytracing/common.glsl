#ifndef COMMONS_DEVICE
#define COMMONS_DEVICE

#include "commons.h"

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
layout(buffer_reference, scalar, buffer_reference_align = 4) readonly buffer EnvSampling { EnvAccel e[]; };


Indices indices = Indices(scene_desc.index_addr);
Vertices vertices = Vertices(scene_desc.vertex_addr);
Normals normals = Normals(scene_desc.normal_addr);
Materials materials = Materials(scene_desc.material_addr);
InstanceInfo prim_infos = InstanceInfo(scene_desc.prim_info_addr);
TexCoords tex_coords = TexCoords(scene_desc.uv_addr);
EnvSampling envSamplingData = EnvSampling(scene_desc.env_sampling_addr);

struct LightSample{
    vec3 indensity;
    vec3 wi;
    float pdf;

    uint triangle_idx;

    vec3 p;
    vec3 n;
    float dist;
    bool is_infinite;
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

float luminance(vec3 v) {
    return 0.2126 * v.x + 0.7152 * v.y + 0.0722 * v.z;
}

bool isBlack(vec3 v){
    return luminance(v) < 1e-6f;
}

MeshSampleRecord uniform_sample_on_mesh(uint mesh_idx, vec3 rands, in mat4 world_matrix,const uint triangle_idx){
    MeshSampleRecord result;
    
    RTPrimitive mesh_info = prim_infos.p[mesh_idx];
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

    result.pdf = 2.f / length(cross(p1 - p0, p2 - p0));


    result.n = normalize(vec3(inv_tr_mat * nrm));
//    result.p = barycentrics.x * p0 + p1 * barycentrics.y + p2 * barycentrics.z;

    vec3 pos =  p0_ * barycentrics.x + p1_ * barycentrics.y + p2_ * barycentrics.z;
    result.p = (world_matrix * vec4(pos, 1.f)).xyz;

    result.triangle_idx = triangle_idx;

    return result;
}


MeshSampleRecord uniform_sample_on_mesh(uint mesh_idx, vec3 rands, in mat4 world_matrix){
    
    RTPrimitive mesh_info = prim_infos.p[mesh_idx];
    float pdf;
    uint triangle_idx = sample_distribution(rands.x, mesh_info.area_distribution_buffer_addr, pdf);

    MeshSampleRecord record = uniform_sample_on_mesh(mesh_idx, rands, world_matrix, triangle_idx);
    record.pdf = 1.f/mesh_info.area;
    
    return record;
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
    
    result.indensity = light.L;

    float pdf;
    //sample one point on primitive 
    MeshSampleRecord record  = uniform_sample_on_mesh(light.prim_idx, rand, light.world_matrix);
    
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

vec2 envdir_to_uv(mat4 toLocal, vec3 wi) {
    vec3 wLocal = (toLocal * vec4(wi, 0)).xyz;
    float uv_x = atan(wLocal.z, wLocal.x) * INV_TWO_PI + 0.5f;
    float uv_y = acos(clamp(-wLocal.y, -1.0, 1.0)) * INV_PI;
    return vec2(uv_x, uv_y);
}




float eval_light_pdf(const RTLight light, const vec3 p,const vec3 light_p, const vec3 n_g){
    if(light.light_type == RT_LIGHT_TYPE_AREA){
        vec3 wi = normalize(light_p - p);
        float cos_theta_light = dot(n_g, -wi);
        if (cos_theta_light <= 0.0){
            return 0;
        }
        float dist = length(light_p - p);
        dist -=  EPS;
        return 1/get_primitive_area(light.prim_idx) * abs(cos_theta_light) / (dist * dist);
    }
    else if(light.light_type == RT_LIGHT_TYPE_INFINITE){
        vec2 uv = envdir_to_uv(light.world_matrix, normalize(light_p - p));
        
    }
    else if(light.light_type == RT_LIGHT_TYPE_POINT){
        return 1.f / (4.f * PI);
    }
    return 0;
}

LightSample sample_li_area_light_with_idx(const RTLight light, const SurfaceScatterEvent event, const vec3 rand,const uint triangle_index){

    LightSample result;

    result.indensity = light.L;

    float pdf;
    //sample one point on primitive 
    MeshSampleRecord record  = uniform_sample_on_mesh(light.prim_idx, rand, light.world_matrix,triangle_index);

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

vec3 Environment_sample(sampler2D lat_long_tex, in vec3 randVal, out vec3 to_light, out float pdf)
{

    // Uniformly pick a texel index idx in the environment map
    vec3  xi     = randVal;
    uvec2 tsize  = textureSize(lat_long_tex, 0);
    uint  width  = tsize.x;
    uint  height = tsize.y;

    const uint size = width * height;
    const uint idx  = min(uint(xi.x * float(size)), size - 1);

    // Fetch the sampling data for that texel, containing the ratio q between its
    // emitted radiance and the average of the environment map, the texel alias,
    // the probability distribution function (PDF) values for that texel and its
    // alias
    EnvAccel sample_data = envSamplingData.e[idx];

    uint env_idx;

    if(xi.y < sample_data.q)
    {
        // If the random variable is lower than the intensity ratio q, we directly pick
        // this texel, and renormalize the random variable for later use. The PDF is the
        // one of the texel itself
        env_idx = idx;
        xi.y /= sample_data.q;
        pdf = sample_data.pdf;
    }
    else
    {
        // Otherwise we pick the alias of the texel, renormalize the random variable and use
        // the PDF of the alias
        env_idx = sample_data.alias;
        xi.y    = (xi.y - sample_data.q) / (1.0f - sample_data.q);
        pdf     = sample_data.aliasPdf;
    }

    // Compute the 2D integer coordinates of the texel
    const uint px = env_idx % width;
    uint       py = env_idx / width;

    // Uniformly sample the solid angle subtended by the pixel.
    // Generate both the UV for texture lookup and a direction in spherical coordinates
    const float u       = float(px + xi.y) / float(width);
    const float phi     = u * (2.0f * PI) - PI;
    float       sin_phi = sin(phi);
    float       cos_phi = cos(phi);

    const float step_theta = PI / float(height);
    const float theta0     = float(py) * step_theta;
    const float cos_theta  = cos(theta0) * (1.0f - xi.z) + cos(theta0 + step_theta) * xi.z;
    const float theta      = acos(cos_theta);
    const float sin_theta  = sin(theta);
    const float v          = theta * INV_PI;

    // Convert to a light direction vector in Cartesian coordinates
    to_light = vec3(cos_phi * sin_theta, cos_theta, sin_phi * sin_theta);

    // Lookup the environment value using bilinear filtering
    return texture(lat_long_tex, vec2(u, v)).xyz;
} 

LightSample sample_li_infinite_light(const RTLight light, const SurfaceScatterEvent event, const vec3 rand){
   
{
    float pdf;
    LightSample result;
    vec3 radiance = Environment_sample( scene_textures[light.light_texture_id], rand, result.wi, pdf);
    result.wi = normalize(mat3(light.world_matrix) * result.wi);
    // Uniformly pick a texel index idx in the environment map
    result.pdf = pdf;
    result.indensity = radiance;
    result.is_infinite = true;
    result.p = event.p + result.wi * 1000.f;
    result.dist = 1000.f;
    return result;
}

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

//LightSample sample_li_with_triangle_idx(const RTLight light, const SurfaceScatterEvent event, const vec3 rand, const uint triangle_idx){
//    LightSample result;
//    
//}



#endif