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




vec3 reflect(const vec3 v){
    return vec3(-v.x, -v.y, v.z);
}

vec3 get_albedo(const RTMaterial mat, const vec2 uv){
    return mat.texture_id == -1 ? mat.albedo : texture(scene_textures[mat.texture_id], uv).rgb;
}

vec3 diffuse_f(const RTMaterial mat, const SurfaceScatterEvent event){
    // return INV_PI * vec3(max(0, get_cos_theta(event.wo)));
    vec3 albedo  = get_albedo(mat, event.uv);
    return albedo * INV_PI * max(0, get_cos_theta(event.wo));
}

BsdfSampleRecord invalid_record(){
    BsdfSampleRecord record;
    record.f = vec3(0);
    record.pdf = 0;
    record.sample_flags = 0;
    return record;
}

BsdfSampleRecord  diffuse_sample(const RTMaterial mat, const vec2 rand, inout SurfaceScatterEvent event){
    BsdfSampleRecord record;
    event.wo = square_to_cosine_hemisphere(rand);

    record.pdf = square_to_cosine_hemisphere_pdf(event.wo);
    record.f = diffuse_f(mat, event);
    record.sample_flags = RT_BSDF_LOBE_DIFFUSE;
    return record;
}

float diffuse_pdf(const RTMaterial mat, const SurfaceScatterEvent event){
    return square_to_cosine_hemisphere_pdf(event.wo);
}


vec3 mirror_f(const RTMaterial mat, const SurfaceScatterEvent event){
    return vec3(0);
}


float mirror_pdf(const RTMaterial mat, const SurfaceScatterEvent event){
    return 0;
}

float conductor_pdf(const RTMaterial mat, const SurfaceScatterEvent event){
    if (event.wi.z <= 0 || event.wo.z <= 0){
        return 0;
    }
    if (mat.roughness == 0){
        return 0;
    }
    else {
        vec3 wh = normalize(event.wi + event.wo);
        return ggx_d(mat.roughness, wh, event.wi) * get_cos_theta(wh) / (4 * dot(wh, event.wo));
    }
}

BsdfSampleRecord conductor_sample(const RTMaterial mat, const vec2 rand, inout SurfaceScatterEvent event){
    BsdfSampleRecord record;
    if (event.wi.z <= 0){
        return invalid_record();
    }
    if (mat.roughness == 0){
        event.wo = reflect(event.wi);
        record.f = conductorReflectanceVec3(mat.eta, mat.k, event.wo.z) * get_albedo(mat, event.uv);
        record.pdf = 1;
        record.sample_flags = RT_BSDF_LOBE_SPECULAR;
        return record;
    }
    else {
        vec3 wh = ggx_sample(mat.roughness, rand);
        event.wo = normalize(event.wi);
        if (dot(event.wi, wh) <= 0){
            return invalid_record();
        }
        record.pdf = ggx_d(mat.roughness, wh, event.wi) * get_cos_theta(wh) / (4 * dot(wh, event.wi));
        float cos_i = dot(wh, event.wi);
        vec3 conductor_fresnel = conductorReflectanceVec3(mat.eta, mat.k, cos_i);
        record.f = get_albedo(mat, event.uv) *  conductor_fresnel * ggx_d(mat.roughness, wh, event.wi) * ggx_g(mat.roughness, event.wi, event.wo, wh) / (4 *  event.wo.z);
        record.sample_flags = RT_BSDF_LOBE_GLOSSY | RT_BSDF_LOBE_REFLECTION;
        return record;
    }
    return invalid_record();
}

vec3 conductor_f(const RTMaterial mat, const SurfaceScatterEvent event){
    if (event.wi.z <= 0 || event.wo.z <= 0){
        return vec3(0);
    }
    if (mat.roughness == 0){
        return vec3(0);
    }
    else {
        vec3 wh = normalize(event.wi + event.wo);
        float cos_i = dot(wh, event.wi);
        vec3 conductor_fresnel = conductorReflectanceVec3(mat.eta, mat.k, cos_i);
        return conductor_fresnel * get_albedo(mat, event.uv) * ggx_d(mat.roughness, wh, event.wi) * ggx_g(mat.roughness, event.wi, event.wo, wh) / (4 * event.wi.z * event.wo.z);
    }
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
HANDLE_BSDF_F(CONDUCTOR,conductor)
return vec3(0);
}

float pdf_bsdf(const RTMaterial mat, const SurfaceScatterEvent event) {
    uint bsdf_type = mat.bsdf_type;
    HANDLE_BSDF_PDF(DIFFUSE,diffuse)
HANDLE_BSDF_PDF(MIRROR,mirror)
HANDLE_BSDF_PDF(CONDUCTOR,conductor)
return 0;
}



BsdfSampleRecord sample_bsdf(const RTMaterial mat, inout SurfaceScatterEvent event, const vec2 rand){
    //Fill event.wo
    //Fill record.f and record.pdf and record.sample_flags
    uint bsdf_type = mat.bsdf_type;
    HANDLE_BSDF_SAMPLE(MIRROR,mirror)
HANDLE_BSDF_SAMPLE(DIFFUSE,diffuse)
HANDLE_BSDF_SAMPLE(CONDUCTOR,conductor)
BsdfSampleRecord record;
return record;
}

#endif