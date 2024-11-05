#ifndef COMMONS_DEVICE
#define COMMONS_DEVICE

#include "commons.h"

#include "util.glsl"


layout(binding = 0, set = 3) uniform accelerationStructureEXT tlas;

layout(binding = 2, set = 0) uniform SceneUboBuffer { SceneUbo scene_ubo; };
layout(binding = 3, set = 0) uniform SceneDescBuffer { SceneDesc scene_desc; };
layout(binding = 4, set = 0) readonly buffer Lights { RTLight lights[]; };
//todo fix this 
layout(binding = 6, set = 1) uniform sampler2D scene_textures[1024];


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


bool is_two_sided(uint material_idx){
    const uint material_type = materials.m[material_idx].bsdf_type;
    return material_type == RT_BSDF_TYPE_DIELECTRIC || material_type == RT_BSDF_TYPE_PRINCIPLE;
}

SurfaceScatterEvent make_surface_scatter_event(HitPayload hit_pay_load, const vec3 wo){
    SurfaceScatterEvent event;

    bool two_sided = is_two_sided(hit_pay_load.material_idx);
    vec3 n_s = hit_pay_load.n_s;
    if (dot(wo, hit_pay_load.n_s) < 0 && !two_sided){
        n_s = -n_s;
    }

    event.frame = make_frame(n_s);
    event.wo = to_local(event.frame, wo);
    event.p = hit_pay_load.p;
    event.material_idx = hit_pay_load.material_idx;
    event.uv = hit_pay_load.uv;
    event.prim_idx = hit_pay_load.prim_idx;
    return event;
}

SurfaceScatterEvent make_surface_scatter_event(vec3 wo, vec3 n, vec3 p, vec2 uv, uint material_idx){
    SurfaceScatterEvent event;

    vec3 n_s = n;
    bool two_sided = is_two_sided(material_idx);
    if (dot(wo, n_s) < 0 && !two_sided){
        n_s = -n_s;
    }
    event.frame = make_frame(n_s);
    event.wo = to_local(event.frame, wo);
    event.p = p;
    event.material_idx = material_idx;
    event.uv = uv;
    return event;
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



MeshSampleRecord uniform_sample_on_mesh(uint mesh_idx, vec3 rands, in mat4 world_matrix, const uint triangle_idx){
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

vec2 envdir_to_uv(mat4 toLocal, vec3 wi) {
    vec3 wLocal = (toLocal * vec4(wi, 0)).xyz;
    float uv_x = atan(wLocal.z, wLocal.x) * INV_TWO_PI + 0.5f;
    float uv_y = acos(clamp(-wLocal.y, -1.0, 1.0)) * INV_PI;
    return vec2(uv_x, uv_y);
}

vec2 envdir_to_uv(mat4 toLocal, vec3 wi, out float sin_theta) {
    vec3 wLocal = (toLocal * vec4(wi, 0)).xyz;
    sin_theta = sqrt(clamp(1 - wLocal.y * wLocal.y, 0, 1));
    if (isnan(sin_theta)){
        debugPrintfEXT("sin_theta %f %f %f %f %f %f\n", wLocal.x, wLocal.y, wLocal.z, wi.x, wi.y, wi.z);
    }
    float uv_x = atan(wLocal.z, wLocal.x) * INV_TWO_PI + 0.5f;
    float uv_y = acos(clamp(-wLocal.y, -1.0, 1.0)) * INV_PI;
    return vec2(uv_x, uv_y);
}


vec3 uv_to_envdir(mat4 toLocal, vec2 uv, out float sin_theta) {
    float phi   = (uv.x - 0.5f) * 2 * PI;
    float theta = uv.y * PI;
    sin_theta    = sin(theta);
    vec3 wLocal = vec3(
    cos(phi) * sin_theta,
    -cos(theta),
    sin(phi) * sin_theta);
    return mat3(toLocal) * wLocal;
}


//Param p: the point on the light source
//Param n_s: the normal of the light source
//Param w: the direction from the point to the light source
//Return: the radiance of the light source
vec3 eval_light(const RTLight light, const vec3 p, const vec3 n_g, const vec3 w){

    if (light.light_type == RT_LIGHT_TYPE_AREA){
        if (dot(n_g, -w) > 0){
            return light.L;
        }
        return vec3(0);
    }
    else if (light.light_type == RT_LIGHT_TYPE_INFINITE){
        vec2 uv = envdir_to_uv(transpose(light.world_matrix), normalize(w));
        return texture(scene_textures[light.light_texture_id], uv).xyz;
    }
    else if (light.light_type == RT_LIGHT_TYPE_POINT){
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
    if (cos_theta_light <= 1e-4f){
        result.n = record.n;
        result.wi = wi;
        result.indensity  = vec3(0);
        result.pdf = 0;
        return result;
    }
    pdf = record.pdf * dist * dist  / abs(cos_theta_light);

//    if(pdf > 1000000000){
//        debugPrintfEXT("pdf %f %f %f %f %f %f %f %f %f\n", pdf, cos_theta_light, dist, light_p.x, light_p.y, light_p.z, p.x, p.y, p.z);
//    }

    result.wi = wi;
    result.n = record.n;
    result.pdf = pdf;
    result.p = light_p;
    result.indensity = light.L;
    result.dist = dist;
    result.triangle_idx = record.triangle_idx;
    return result;
}




float eval_light_pdf(const RTLight light, const vec3 p, const vec3 light_p, const vec3 n_g, const vec3 wi){
    if (light.light_type == RT_LIGHT_TYPE_AREA){
        float cos_theta_light = dot(n_g, -wi);
        if (cos_theta_light <= 0.0){
            return 0;
        }
        float dist = length(light_p - p);
        dist -=  EPS;
        float pdf =  1/get_primitive_area(light.prim_idx) * dist * dist  / abs(cos_theta_light);
        if (isnan(pdf)){
            debugPrintfEXT("pdf %f %f %f %f %f %f %f %f %f\n", pdf, cos_theta_light, dist, light_p.x, light_p.y, light_p.z, p.x, p.y, p.z);
        }
        return pdf;
    }
    else if (light.light_type == RT_LIGHT_TYPE_INFINITE){
        float sin_theta;
        vec2 uv = envdir_to_uv(transpose(light.world_matrix), wi, sin_theta);
        if (sin_theta <= 1e-4f){
            return 0;
        }
        uvec2 texture_size = textureSize(scene_textures[light.light_texture_id], 0);
        uvec2 texture_pos = uvec2(uint(texture_size.x * uv.x), uint(texture_size.y * uv.y));
        uint pixel_index = texture_pos.y * texture_size.x + texture_pos.x;
        float pdf = envSamplingData.e[pixel_index].pdf *  INV_PI * INV_TWO_PI/ sin_theta;
        if (isnan(pdf)){
            debugPrintfEXT("pdf sin_theta %f %f %d %d\n", envSamplingData.e[pixel_index].pdf, sin_theta, texture_pos.x, texture_pos.y);
        }
        return envSamplingData.e[pixel_index].pdf *  INV_PI * INV_TWO_PI/ sin_theta;
    }
    else if (light.light_type == RT_LIGHT_TYPE_POINT){
        return 1.f / (4.f * PI);
    }
    else if (light.light_type == RT_LIGHT_TYPE_DIRECTIONAL){
        //delta light
        return 0;
    }
    return 0;
}

LightSample sample_li_area_light_with_idx(const RTLight light, const SurfaceScatterEvent event, const vec3 rand, const uint triangle_index){

    LightSample result;

    result.indensity = light.L;

    float pdf;
    //sample one point on primitive 
    MeshSampleRecord record  = uniform_sample_on_mesh(light.prim_idx, rand, light.world_matrix, triangle_index);

    vec3 light_p = record.p;
    vec3 p = event.p;

    vec3 wi = light_p - p;
    float dist = length(wi);

    wi /= dist;

    dist -=  EPS;

    float cos_theta_light = dot(record.n, -wi);
    if (cos_theta_light <= 1e-4f){
        result.n = record.n;
        result.wi = wi;
        result.indensity  = vec3(0);
        result.pdf = 0;
        return result;
    }
    pdf = record.pdf * dist * dist  / abs(cos_theta_light);
    
//    if(pdf > 1000000000){
//        debugPrintfEXT("pdf %f %f %f %f %f %f %f %f %f\n", pdf, cos_theta_light, dist, light_p.x, light_p.y, light_p.z, p.x, p.y, p.z);
//    }

    result.wi = wi;
    result.n = record.n;
    result.pdf = pdf;
    result.p = light_p;
    result.indensity = light.L;
    result.dist = dist;
    result.triangle_idx = record.triangle_idx;

    return result;
}

vec3 Environment_sample(sampler2D lat_long_tex, mat4 matrix, in vec3 randVal, out vec3 to_light, out float pdf)
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

    if (xi.y < sample_data.q)
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
    const float v = float(py + xi.z) / float(height);


    float sin_theta;
    // Convert to a light direction vector in Cartesian coordinates
    to_light = uv_to_envdir(matrix, vec2(u, v), sin_theta);

    if (sin_theta <= 1e-4f)
    {
        // If the sin_theta is zero, the PDF is zero
        pdf = 0.0f;
        return vec3(0.0f);
    }
    pdf = pdf * width * height / (2.0f * PI * PI * sin_theta);

    //    debugPrintfEXT("pdf %f\n", pdf);

    // Lookup the environment value using bilinear filtering
    return texture(lat_long_tex, vec2(u, v)).xyz;
}

LightSample sample_li_infinite_light(const RTLight light, const SurfaceScatterEvent event, const vec3 rand){

    {
        float pdf;
        LightSample result;
        vec3 radiance = Environment_sample(scene_textures[light.light_texture_id], light.world_matrix, rand, result.wi, pdf);
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

LightSample sample_li_directional_light(const RTLight light, const SurfaceScatterEvent event, const vec3 rand){
    LightSample result;
    result.indensity = light.L;
    result.wi = normalize(-light.direction);
    result.pdf = 1.f;
    result.p = event.p + result.wi * 1000.f;
    result.dist = 1000.f;
    return result;
}


LightSample sample_li(const RTLight light, const SurfaceScatterEvent event, const vec3 rand){
    uint light_type = light.light_type;
    LightSample result;

    if (light_type == RT_LIGHT_TYPE_AREA){
        result =  sample_li_area_light(light, event, rand);
        result.is_infinite = false;

    }
    else if (light_type == RT_LIGHT_TYPE_INFINITE){
        result =  sample_li_infinite_light(light, event, rand);
        result.is_infinite = true;
    }
    else if (light_type == RT_LIGHT_TYPE_POINT){
        result =  sample_li_point_light(light, event, rand);
        result.is_infinite = false;
    }
    else if (light_type == RT_LIGHT_TYPE_DIRECTIONAL){
        result =  sample_li_directional_light(light, event, rand);
        result.is_infinite = true;
    }
    return result;
}

bool is_specular_material(const RTMaterial material){
    if (material.bsdf_type == RT_BSDF_TYPE_DIELECTRIC || material.bsdf_type == RT_BSDF_TYPE_CONDUCTOR || material.bsdf_type == RT_BSDF_TYPE_PLASTIC || material.bsdf_type == RT_BSDF_TYPE_MIRROR){
        return material.roughness < 1e-3;
    }
    return false;
}

//LightSample sample_li_with_triangle_idx(const RTLight light, const SurfaceScatterEvent event, const vec3 rand, const uint triangle_idx){
//    LightSample result;
//    
//}



#endif