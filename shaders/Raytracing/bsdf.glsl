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




vec3 my_reflect(const vec3 v){
    return vec3(-v.x, -v.y, v.z);
}

vec3 my_reflect(vec3 v, vec3 n){
    return -v + 2 * dot(v, n) * n;
}


vec3 get_albedo(const RTMaterial mat, const vec2 uv){
    return mat.texture_id == -1 ? mat.albedo : texture(scene_textures[mat.texture_id], uv).rgb;
}

vec3 diffuse_f(const RTMaterial mat, const SurfaceScatterEvent event){
    // return INV_PI * vec3(max(0, get_cos_theta(event.wo)));
    vec3 albedo  = get_albedo(mat, event.uv);
    return albedo * INV_PI * max(0, get_cos_theta(event.wi));
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
    if (event.wi.z <= 0 || event.wo.z <= 0){
        return 0;
    }
    if (mat.roughness <1e-3f){
        return 0;
    }
    else {
        vec3 wh = normalize(event.wi + event.wo);
        return ggx_d(mat.roughness, wh, event.wi) * get_cos_theta(wh) / (4 * dot(wh, event.wo));
    }
}


BsdfSampleRecord plastic_sample(const RTMaterial mat, const vec2 rand, inout SurfaceScatterEvent event){

    //    debugPrintfEXT("Plastic Sample\n");
    BsdfSampleRecord record;

    if (mat.roughness>0){
        if (get_cos_theta(event.wo) <= 0) {
            return invalid_record();
        }
        float FOut = dielectricReflectance(1 / mat.ior, event.wo.z);

        float glossyProb = FOut;
        float diffProb = (1 - glossyProb) * mat.avgTransmittance;
        glossyProb /= (glossyProb + diffProb);



        vec3 wh;
        if (rand.x < glossyProb) {
            float remapU0 = (glossyProb - rand.x) / glossyProb;
            vec2 newU = vec2(remapU0, rand.y);
            wh = ggx_sample(mat.roughness, newU);
            event.wi = my_reflect(event.wo, wh);
            if (event.wi.z <= 0)
            return invalid_record();
            record.sample_flags = RT_BSDF_LOBE_GLOSSY | RT_BSDF_LOBE_REFLECTION;
            record.pdf = glossyProb * ggx_d(mat.roughness, wh, event.wo) / (4 * abs(dot(event.wo, wh))) +
            (1 - glossyProb) * event.wi.z / PI;
        } else {
            float remapU0 = (rand.x - glossyProb) / (1 - glossyProb);
            vec2 newU = vec2(remapU0, rand.y);
            event.wi = square_to_cosine_hemisphere(newU);
            if (event.wi.z <= 0)
            return invalid_record();
            record.sample_flags = RT_BSDF_LOBE_DIFFUSE | RT_BSDF_LOBE_REFLECTION;
            wh = normalize((event.wi + event.wo));
            record.pdf = glossyProb * ggx_d(mat.roughness, wh, event.wo) / (4 * abs(dot(event.wo, wh))) +
            (1 - glossyProb) * event.wi.z / PI;
        }
        vec3 albedo = get_albedo(mat, event.uv);
        float D = ggx_d(mat.roughness, wh, event.wo);
        float G = ggx_g(mat.roughness, event.wi, event.wo, wh);
        vec3 specularContrib = vec3(FOut * D * G / (4 * event.wo.z));
        float FIn = dielectricReflectance(1 / mat.ior, event.wo.z);
        vec3 diffuseContrib =
        albedo * (1 - FOut) * (1 - FIn) * (1 / (mat.ior * mat.ior)) * abs(get_cos_theta(event.wi)) / (vec3(1) - mat.diffuseFresnel * albedo) / PI;
        if (all(greaterThan(mat.scaledSigmaA, vec3(0))))
        diffuseContrib *= exp(mat.scaledSigmaA * (-1.0f / event.wo.z - 1.0f / event.wi.z));
        record.f = specularContrib + diffuseContrib;
        return record;
    }
    else {

    }

    return invalid_record();
}

vec3 plastic_f(const RTMaterial mat, const SurfaceScatterEvent event){
    if (mat.roughness>0){
        {
            const vec3 wo = event.wo;
            const vec3 wi = event.wi;
            if (wo.z <= 0 || wi.z <= 0) {
                return vec3(0);
            }
            vec3 wh = normalize((wo + wi));
            float FOut = dielectricReflectance(1 / mat.ior, dot(wo, wh));
            float D = ggx_d(mat.roughness, wh, wi);
            float G = ggx_g(mat.roughness, wi, wo, wh);
            vec3 specularContrib = vec3(FOut * D * G / (4 * wo.z));

            float FIn = dielectricReflectance(1 / mat.ior, dot(wi, wh));
            vec3 albedo = get_albedo(mat, event.uv);
            vec3 diffuseContrib =
            albedo * (1 - FOut) * (1 - FIn) * (1 / (mat.ior * mat.ior)) * abs(get_cos_theta(wi)) * INV_PI / (vec3(1) - mat.diffuseFresnel * albedo);
            if (all(greaterThan(mat.scaledSigmaA, vec3(0))))
            diffuseContrib *= exp(mat.scaledSigmaA * (-1.0f / wo.z - 1.0f / wi.z));
            return specularContrib + diffuseContrib;
        }
    }
    return vec3(0);
}

float plastic_pdf(const RTMaterial mat, const SurfaceScatterEvent event){
    if (mat.roughness>0){
        const vec3 wo = event.wo;
        const vec3 wi = event.wi;
        if (get_cos_theta(wo) <= 0 || get_cos_theta(wi) <= 0)
        return 0;
        vec3 wh = normalize(wo + wi);
        float F = dielectricReflectance(1 / mat.ior, wo.z);
        float glossyProb = F;
        float diffProb = (1 - glossyProb) * mat.avgTransmittance;
        glossyProb /= (glossyProb + diffProb);
        diffProb = 1 - glossyProb;
        float D = ggx_d(mat.roughness, wh, wi);
        glossyProb *= D / (4 * abs(dot(wo, wh)));
        diffProb *= wi.z / PI;
        return glossyProb + diffProb;
    }
    return 1;
}

BsdfSampleRecord conductor_sample(const RTMaterial mat, const vec2 rand, inout SurfaceScatterEvent event){
    BsdfSampleRecord record;
    if (event.wo.z <= 0){
        return invalid_record();
    }
    if (mat.roughness <1e-3f){
        event.wi = my_reflect(event.wo);
        record.f = conductorReflectanceVec3(mat.eta, mat.k, event.wo.z) * get_albedo(mat, event.uv);
        record.pdf = 1;
        record.sample_flags = RT_BSDF_LOBE_SPECULAR | RT_BSDF_LOBE_REFLECTION;
        return record;
    }
    else {
        vec3 wh = ggx_sample(mat.roughness, rand);
        event.wi = my_reflect(event.wo, wh);
        if (dot(event.wi, wh) <= 0){
            // debugPrintfEXT("Invalid Sample wi %f %f %f wo %f %f %f wh %f %f %f\n", event.wi.x, event.wi.y, event.wi.z, event.wo.x, event.wo.y, event.wo.z, wh.x, wh.y, wh.z);
            return invalid_record();
        }
        record.pdf = ggx_d(mat.roughness, wh, event.wo) * get_cos_theta(wh) / (4 * dot(wh, event.wo));
        float cos_o = dot(wh, event.wo);
        vec3 conductor_fresnel = conductorReflectanceVec3(mat.eta, mat.k, cos_o);
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
    if (mat.roughness <= 1e-3f){
        return vec3(0);
    }
    else {
        vec3 wh = normalize(event.wi + event.wo);
        float cos_o = dot(wh, event.wo);
        vec3 conductor_fresnel = conductorReflectanceVec3(mat.eta, mat.k, cos_o);
        return conductor_fresnel * get_albedo(mat, event.uv) * ggx_d(mat.roughness, wh, event.wi) * ggx_g(mat.roughness, event.wi, event.wo, wh) / (4 * event.wo.z);
    }
}


BsdfSampleRecord mirror_sample(const RTMaterial mat, const vec2 rand, inout SurfaceScatterEvent event){
    BsdfSampleRecord record;
    record.f = get_albedo(mat, event.uv);
    record.pdf = 1;
    record.sample_flags = RT_BSDF_LOBE_SPECULAR;
    event.wi = my_reflect(event.wo, vec3(0, 0, 1));
    return record;
}


vec3 eval_bsdf(const RTMaterial mat, const SurfaceScatterEvent event) {
    uint bsdf_type = mat.bsdf_type;
    HANDLE_BSDF_F(DIFFUSE,diffuse)
HANDLE_BSDF_F(MIRROR,mirror)
HANDLE_BSDF_F(CONDUCTOR,conductor)
HANDLE_BSDF_F(PLASTIC,plastic)
return vec3(0);
}

float pdf_bsdf(const RTMaterial mat, const SurfaceScatterEvent event) {
    uint bsdf_type = mat.bsdf_type;
    HANDLE_BSDF_PDF(DIFFUSE,diffuse)
HANDLE_BSDF_PDF(MIRROR,mirror)
HANDLE_BSDF_PDF(CONDUCTOR,conductor)
HANDLE_BSDF_PDF(PLASTIC,plastic)
return 0;
}



BsdfSampleRecord sample_bsdf(const RTMaterial mat, inout SurfaceScatterEvent event, const vec2 rand){
    //Fill event.wo
    //Fill record.f and record.pdf and record.sample_flags
    uint bsdf_type = mat.bsdf_type;
    HANDLE_BSDF_SAMPLE(MIRROR,mirror)
HANDLE_BSDF_SAMPLE(DIFFUSE,diffuse)
HANDLE_BSDF_SAMPLE(CONDUCTOR,conductor)
HANDLE_BSDF_SAMPLE(PLASTIC,plastic)
BsdfSampleRecord record;
return record;
}

#endif