#ifndef COMMONS_GLSL
#define COMMONS_GLSL

#include "commons.h"
#include "util.glsl"

#define HANDLE_BSDF_SAMPLE(BSDF_NAME_UPPER, BSDF_NAME_LOWER) if(bsdf_type == RT_BSDF_TYPE_##BSDF_NAME_UPPER){ return BSDF_NAME_LOWER##_sample(mat, rand, event); }


#define HANDLE_BSDF_PDF(BSDF_NAME_UPPER, BSDF_NAME_LOWER) \
    if(bsdf_type == RT_BSDF_TYPE_##BSDF_NAME_UPPER){ \
 return BSDF_NAME_LOWER##_pdf(mat, event);\
 }

#define HANDLE_BSDF_F(BSDF_NAME_UPPER, BSDF_NAME_LOWER) \
    if(bsdf_type == RT_BSDF_TYPE_##BSDF_NAME_UPPER){ \
 return BSDF_NAME_LOWER##_f(mat, event);\
 }

//vec3 reflect(const vec3 v,const vec3 n){
//    return v - 2 * dot(v,n) * n;
//}

vec3 reflect(const vec3 v){
    return vec3(-v.x, -v.y, v.z);
}

vec3 get_albedo(const RTMaterial mat, const vec2 uv){
    return mat.texture_id == -1 ? mat.albedo : texture(scene_textures[mat.texture_id], uv).rgb;
}

vec3 diffuse_f(const RTMaterial mat, const SurfaceScatterEvent event){
    // return INV_PI * vec3(max(0, get_cos_theta(event.wi)));
    vec3 albedo  = get_albedo(mat, event.uv);
    return albedo * INV_PI * max(0, get_cos_theta(event.wi));
}

BsdfSampleRecord  diffuse_sample(const RTMaterial mat, const vec2 rand, inout SurfaceScatterEvent event){
    BsdfSampleRecord record;
    event.wi = square_to_cosine_hemisphere(rand);

    record.pdf = square_to_cosine_hemisphere_pdf(event.wi);
    record.f = diffuse_f(mat, event);
    record.sample_flags = RT_BSDF_LOBE_DIFFUSE;
    return record;
}

float diffuse_pdf(const RTMaterial mat, const SurfaceScatterEvent event){
    return square_to_cosine_hemisphere_pdf(event.wi);
}


vec3 mirror_f(const RTMaterial mat, const SurfaceScatterEvent event){
    return vec3(0);
}


float mirror_pdf(const RTMaterial mat, const SurfaceScatterEvent event){
    return 0;
}

float conductor_pdf(const RTMaterial mat, const SurfaceScatterEvent event){
    return 0;
}

BsdfSampleRecord conductor_sample(const RTMaterial mat, const vec2 rand, inout SurfaceScatterEvent event){
    BsdfSampleRecord record;
    if (mat.roughness == 0){
        event.wi = reflect(event.wo);
        record.f = conductorReflectance(mat.eta, mat.k, event.wo.z);
        record.pdf = 1;
        record.sample_flags = RT_BSDF_LOBE_SPECULAR;
        return record;
    }
}

vec3 conductor_f(const RTMaterial mat, const SurfaceScatterEvent event){
    return vec3(0);
}


BsdfSampleRecord mirror_sample(const RTMaterial mat, const vec2 rand, inout SurfaceScatterEvent event){
    BsdfSampleRecord record;
    record.f = get_albedo(mat, event.uv);
    record.pdf = 1;
    record.sample_flags = RT_BSDF_LOBE_SPECULAR;
    return record;
}


vec3 eval_bsdf(const RTMaterial mat, const SurfaceScatterEvent event) {
    uint bsdf_type = mat.bsdf_type;
    HANDLE_BSDF_F(DIFFUSE,diffuse)
HANDLE_BSDF_F(MIRROR,mirror)
return vec3(0);
}

float pdf_bsdf(const RTMaterial mat, const SurfaceScatterEvent event) {
    uint bsdf_type = mat.bsdf_type;
    HANDLE_BSDF_PDF(DIFFUSE,diffuse)
HANDLE_BSDF_PDF(MIRROR,mirror)
return 0;
}



BsdfSampleRecord sample_bsdf(const RTMaterial mat, inout SurfaceScatterEvent event, const vec2 rand){
    //Fill event.wi
    //Fill record.f and record.pdf and record.sample_flags
    uint bsdf_type = mat.bsdf_type;
    HANDLE_BSDF_SAMPLE(MIRROR,mirror)
HANDLE_BSDF_SAMPLE(DIFFUSE,diffuse)
BsdfSampleRecord record;
return record;
}

#endif