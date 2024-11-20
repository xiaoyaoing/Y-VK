#ifndef COMMONS_GLSL
#define COMMONS_GLSL

#include "commons.h"
#include "util.glsl"

#define HANDLE_BSDF_SAMPLE(BSDF_NAME_UPPER, BSDF_NAME_LOWER) if(bsdf_type == RT_BSDF_TYPE_##BSDF_NAME_UPPER){ return BSDF_NAME_LOWER##_sample(mat, seed, event); }


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

float get_roughness(const RTMaterial mat, const vec2 uv){
    return mat.roughness_texture_id == -1 ? mat.roughness : texture(scene_textures[mat.roughness_texture_id], uv).r;
}

float get_metallness(const RTMaterial mat, const vec2 uv){
    return mat.roughness_texture_id == -1 ? mat.metallic : texture(scene_textures[mat.roughness_texture_id], uv).g;
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

BsdfSampleRecord  diffuse_sample(const RTMaterial mat, inout uvec4 seed, inout SurfaceScatterEvent event){
    const vec2 rand = rand2(seed);
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
    float roughness = get_roughness(mat, event.uv);
    if (roughness <1e-3f){
        return 0;
    }
    else {
        vec3 wh = normalize(event.wi + event.wo);
        return ggx_d(roughness, wh, event.wi) * get_cos_theta(wh) / (4 * dot(wh, event.wo));
    }
}

float conductor_albedo_pdf(const RTMaterial mat, const SurfaceScatterEvent event){
    if (event.wi.z <= 0 || event.wo.z <= 0){
        return 0;
    }
    float roughness = get_roughness(mat, event.uv);
    if (roughness <1e-3f){
        return 0;
    }
    else {
        vec3 wh = normalize(event.wi + event.wo);
        return ggx_d(roughness, wh, event.wi) * get_cos_theta(wh) / (4 * dot(wh, event.wo));
    }
}

BsdfSampleRecord conductor_albedo_sample(const RTMaterial mat, inout uvec4 seed, inout SurfaceScatterEvent event){
    const vec2 rand = rand2(seed);
    BsdfSampleRecord record;
    if (event.wo.z <= 0){
        return invalid_record();
    }
    float roughness = get_roughness(mat, event.uv);
    if (roughness <1e-3f){
        event.wi = my_reflect(event.wo);
        record.f = fresnelSchlick(get_albedo(mat, event.uv), event.wo.z);
        record.pdf = 1;
        record.sample_flags = RT_BSDF_LOBE_SPECULAR | RT_BSDF_LOBE_REFLECTION;
        return record;
    }
    else {
        vec3 wh = ggx_sample(roughness, rand);
        event.wi = my_reflect(event.wo, wh);
        if (dot(event.wi, wh) <= 0){
            return invalid_record();
        }
        record.pdf = ggx_d(roughness, wh, event.wo) * get_cos_theta(wh) / (4 * dot(wh, event.wo));
        float cos_o = dot(wh, event.wo);
        vec3 conductor_fresnel = fresnelSchlick(get_albedo(mat, event.uv), cos_o);
        record.f =   conductor_fresnel * ggx_d(roughness, wh, event.wo) * ggx_g(roughness, event.wi, event.wo, wh) / (4 *  event.wo.z);
        record.sample_flags = RT_BSDF_LOBE_GLOSSY | RT_BSDF_LOBE_REFLECTION;
        return record;
    }
    return invalid_record();
}

vec3 conductor_albedo_f(const RTMaterial mat, const SurfaceScatterEvent event){
    if (event.wi.z <= 0 || event.wo.z <= 0){
        return vec3(0);
    }
    float roughness = get_roughness(mat, event.uv);
    if (roughness <= 1e-3f){
        return vec3(0);
    }
    else {
        vec3 wh = normalize(event.wi + event.wo);
        float cos_o = dot(wh, event.wo);
        vec3 conductor_fresnel = fresnelSchlick(get_albedo(mat, event.uv), cos_o);
        vec3 result =  conductor_fresnel * ggx_d(roughness, wh, event.wo) * ggx_g(roughness, event.wi, event.wo, wh) / (4 * event.wo.z);
        if(has_nan(result)){
            debugPrintfEXT("conductor_fresnel %f %f %f\n", conductor_fresnel.x, conductor_fresnel.y, conductor_fresnel.z);
            debugPrintfEXT("ggx_d %f\n", ggx_d(roughness, wh, event.wo));
            debugPrintfEXT("ggx_g %f\n", ggx_g(roughness, event.wi, event.wo, wh));
            debugPrintfEXT("event.wo.z %f\n", event.wo.z);
        }
    }
}


BsdfSampleRecord plastic_sample(const RTMaterial mat, inout uvec4 seed, inout SurfaceScatterEvent event){
    const vec2 rand = rand2(seed);

    //    debugPrintfEXT("Plastic Sample\n");
    BsdfSampleRecord record;
    float roughness = get_roughness(mat, event.uv);
    if (roughness>0){
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
            wh = ggx_sample(roughness, newU);
            event.wi = my_reflect(event.wo, wh);
            if (event.wi.z <= 0)
            return invalid_record();
            record.sample_flags = RT_BSDF_LOBE_GLOSSY | RT_BSDF_LOBE_REFLECTION;
            record.pdf = glossyProb * ggx_d(roughness, wh, event.wo) / (4 * abs(dot(event.wo, wh))) +
            (1 - glossyProb) * event.wi.z / PI;
        } else {
            float remapU0 = (rand.x - glossyProb) / (1 - glossyProb);
            vec2 newU = vec2(remapU0, rand.y);
            event.wi = square_to_cosine_hemisphere(newU);
            if (event.wi.z <= 0)
            return invalid_record();
            record.sample_flags = RT_BSDF_LOBE_DIFFUSE | RT_BSDF_LOBE_REFLECTION;
            wh = normalize((event.wi + event.wo));
            record.pdf = glossyProb * ggx_d(roughness, wh, event.wo) / (4 * abs(dot(event.wo, wh))) +
            (1 - glossyProb) * event.wi.z / PI;
        }
        vec3 albedo = get_albedo(mat, event.uv);
        float D = ggx_d(roughness, wh, event.wo);
        float G = ggx_g(roughness, event.wi, event.wo, wh);
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
    float roughness = get_roughness(mat, event.uv);
    if (roughness>0){
        const vec3 wo = event.wo;
        const vec3 wi = event.wi;
        if (wo.z <= 0 || wi.z <= 0) {
            return vec3(0);
        }
        vec3 wh = normalize((wo + wi));
        float FOut = dielectricReflectance(1 / mat.ior, dot(wo, wh));
        float D = ggx_d(roughness, wh, wo);
        float G = ggx_g(roughness, wi, wo, wh);
        vec3 specularContrib = vec3(FOut * D * G / (4 * wo.z));

        float FIn = dielectricReflectance(1 / mat.ior, dot(wi, wh));
        vec3 albedo = get_albedo(mat, event.uv);
        vec3 diffuseContrib =
        albedo * (1 - FOut) * (1 - FIn) * (1 / (mat.ior * mat.ior)) * abs(get_cos_theta(wi)) * INV_PI / (vec3(1) - mat.diffuseFresnel * albedo);
        if (all(greaterThan(mat.scaledSigmaA, vec3(0))))
        diffuseContrib *= exp(mat.scaledSigmaA * (-1.0f / wo.z - 1.0f / wi.z));
        return specularContrib + diffuseContrib;
    }
    return vec3(0);
}

float plastic_pdf(const RTMaterial mat, const SurfaceScatterEvent event){
    float roughness = get_roughness(mat, event.uv);
    if (roughness>0){
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
        float D = ggx_d(roughness, wh, wo);
        glossyProb *= D / (4 * abs(dot(wo, wh)));
        diffProb *= wi.z / PI;
        return glossyProb + diffProb;
    }
    return 0;
}

BsdfSampleRecord conductor_sample(const RTMaterial mat, inout uvec4 seed, inout SurfaceScatterEvent event){
    float roughness = get_roughness(mat, event.uv);
    const vec2 rand = rand2(seed);
    BsdfSampleRecord record;
    if (event.wo.z <= 0){
        return invalid_record();
    }
    if (roughness <1e-3f){
        event.wi = my_reflect(event.wo);
        record.f = conductorReflectanceVec3(mat.eta, mat.k, event.wo.z) * get_albedo(mat, event.uv);
        record.pdf = 1;
        record.sample_flags = RT_BSDF_LOBE_SPECULAR | RT_BSDF_LOBE_REFLECTION;
        return record;
    }
    else {
        vec3 wh = ggx_sample(roughness, rand);
        event.wi = my_reflect(event.wo, wh);
        if (dot(event.wi, wh) <= 0){
            // debugPrintfEXT("Invalid Sample wi %f %f %f wo %f %f %f wh %f %f %f\n", event.wi.x, event.wi.y, event.wi.z, event.wo.x, event.wo.y, event.wo.z, wh.x, wh.y, wh.z);
            return invalid_record();
        }
        record.pdf = ggx_d(roughness, wh, event.wo) * get_cos_theta(wh) / (4 * dot(wh, event.wo));
        float cos_o = dot(wh, event.wo);
        vec3 conductor_fresnel = conductorReflectanceVec3(mat.eta, mat.k, cos_o);
        record.f = get_albedo(mat, event.uv) *  conductor_fresnel * ggx_d(roughness, wh, event.wo) * ggx_g(roughness, event.wi, event.wo, wh) / (4 *  event.wo.z);
        record.sample_flags = RT_BSDF_LOBE_GLOSSY | RT_BSDF_LOBE_REFLECTION;
        return record;
    }
    return invalid_record();
}

vec3 conductor_f(const RTMaterial mat, const SurfaceScatterEvent event){
    float roughness = get_roughness(mat, event.uv);
    if (event.wi.z <= 0 || event.wo.z <= 0){
        return vec3(0);
    }
    if (roughness <= 1e-3f){
        return vec3(0);
    }
    else {
        vec3 wh = normalize(event.wi + event.wo);
        float cos_o = dot(wh, event.wo);
        vec3 conductor_fresnel = conductorReflectanceVec3(mat.eta, mat.k, cos_o);
        return conductor_fresnel * get_albedo(mat, event.uv) * ggx_d(roughness, wh, event.wo) * ggx_g(roughness, event.wi, event.wo, wh) / (4 * event.wo.z);
    }
}


BsdfSampleRecord mirror_sample(const RTMaterial mat, inout uvec4 seed, inout SurfaceScatterEvent event){
    const vec2 rand = rand2(seed);
    BsdfSampleRecord record;
    record.f = get_albedo(mat, event.uv);
    record.pdf = 1;
    record.sample_flags = RT_BSDF_LOBE_SPECULAR;
    event.wi = my_reflect(event.wo, vec3(0, 0, 1));
    return record;
}

float copy_sign(float value, float sign_v){
    return sign(sign_v) * value;
}


float dielectric_pdf(const RTMaterial mat, const SurfaceScatterEvent event) {
    float roughness = get_roughness(mat, event.uv);
    if (roughness < 1e-3f) {
        return 0.0;
    } else {
        vec3 outgoing = event.wo;
        vec3 incoming = event.wi;
        bool reflect = outgoing.z * incoming.z > 0;
        float eta = outgoing.z > 0 ? 1.0 / mat.ior : mat.ior;
        vec3 wh;
        float pdf;
        if (reflect) {
            wh = normalize(incoming + outgoing) * sign(outgoing.z);
        } else {
            wh = -normalize(incoming + outgoing * eta);
        }
        float F = dielectricReflectance(1.0 / mat.ior, dot(outgoing, wh));
        float whPdf = ggx_d(roughness, wh,outgoing);
        if (whPdf < 1e-50) {
            return 0.0;
        }
        if (reflect) {
            pdf = F * whPdf / (4.0 * abs(dot(outgoing, wh)));
            if(isnan(pdf)){
                debugPrintfEXT("roughness wh outgoing %f %f %f %f %f %f\n", roughness, wh.x, wh.y, wh.z, outgoing.x, outgoing.y, outgoing.z);
            }
        } else {
            float sqrtDenom = dot(outgoing, wh) * eta + dot(incoming, wh);
            float dWhDWi = abs(dot(incoming, wh)) / (sqrtDenom * sqrtDenom);
            pdf = whPdf * (1.0 - F) * dWhDWi;
            if(isnan(pdf)){
                    debugPrintfEXT("roughness sqrt outgoing incoming %f %f %f %f %f %f %f %f %f \n", roughness, sqrtDenom, outgoing.x, outgoing.y, outgoing.z, incoming.x, incoming.y, incoming.z, dWhDWi);
                  debugPrintfEXT("whPdf F wh %f %f %f %f %f \n", whPdf, F, wh.x, wh.y, wh.z);
            }
        }
        
        return pdf;
    }
}

float getEtaScale(const RTMaterial mat,in vec3 wo, in vec3 wi){
    if(dot(wo, wi) > 0){
        return 1;
    }
    else{
        return pow(wo.z > 0 ? 1.0 / mat.ior : mat.ior,2);
    }
}

vec3 dielectric_f(const RTMaterial mat, const SurfaceScatterEvent event) {
    float roughness = get_roughness(mat, event.uv);
    if (roughness < 1e-3f) {
        return vec3(0);
    } else {
        vec3 albedo = get_albedo(mat, event.uv);
        vec3 outgoing = event.wo;
        vec3 incoming = event.wi;
        bool reflect = outgoing.z * incoming.z > 0;
        float eta = outgoing.z > 0 ? 1.0 / mat.ior : mat.ior;
        vec3 wh;
        if (reflect) {
            wh = sign(outgoing.z) * normalize(incoming + outgoing);
        } else {
            wh = -normalize(incoming + outgoing * eta);
        }
        float F = dielectricReflectance(1.0 / mat.ior, dot(outgoing, wh));

        float D = ggx_d(roughness, wh, outgoing);
        float G = ggx_g(roughness, incoming, outgoing, wh);
        if (reflect) {
            return albedo * F * D * G / (4.0 * abs(outgoing.z)) * getEtaScale(mat, outgoing, incoming);
        } else {
            float whDotIn = dot(wh, incoming);
            float whDotOut = dot(wh, outgoing);
            float sqrtDenom = eta * whDotOut + whDotIn;
            return albedo * (1.0 - F) * D * G * abs(whDotIn * whDotOut / (outgoing.z * sqrtDenom * sqrtDenom)) * getEtaScale(mat, outgoing, incoming);
        }
    }
}


BsdfSampleRecord dielectric_sample(const RTMaterial mat, inout uvec4 seed, inout SurfaceScatterEvent event) {
    float roughness = get_roughness(mat, event.uv);
    const vec2 rand = rand2(seed);
    BsdfSampleRecord record;
    float eta = event.wo.z < 0 ? mat.ior : 1.0 / mat.ior;

    if (roughness < 1e-3f) {
        float costhetaT;
        float fresnel = dielectricReflectance(eta, event.wo.z, costhetaT);
        if (rand.x < fresnel) {
            event.wi = my_reflect(event.wo, vec3(0, 0, 1));
            record.f = vec3(fresnel) * get_albedo(mat, event.uv);
            record.pdf = fresnel;
            record.sample_flags = RT_BSDF_LOBE_SPECULAR | RT_BSDF_LOBE_REFLECTION;
        } else {
            event.wi = vec3(-eta * event.wo.x, -eta * event.wo.y, -copy_sign(costhetaT, event.wo.z));
            record.f = vec3(1 - fresnel) * get_albedo(mat, event.uv) * getEtaScale(mat, event.wo, event.wi);
            record.pdf = 1 - fresnel;
            record.sample_flags = RT_BSDF_LOBE_SPECULAR | RT_BSDF_LOBE_REFRACTION;
        }
    } else {
        vec3 wh = ggx_sample(roughness, rand);
        float whDotOut = dot(event.wo, wh);
        float cosThetaT;
        float F = dielectricReflectance(1.0 / mat.ior, whDotOut, cosThetaT);
        bool reflect = rand1(seed) < F;
        if (reflect) {
            vec3 incoming = -event.wo + 2.0 * dot(event.wo, wh) * wh;
            event.wi = incoming;
            record.sample_flags = RT_BSDF_LOBE_GLOSSY | RT_BSDF_LOBE_REFLECTION;
        } else {
            float eta = whDotOut < 0.0 ? mat.ior : 1.0 / mat.ior;
            vec3 incoming = (eta * whDotOut - sign(whDotOut) * cosThetaT) * wh - eta * event.wo;
            event.wi = incoming;
            record.sample_flags = RT_BSDF_LOBE_GLOSSY | RT_BSDF_LOBE_REFRACTION;
        }
        if(hasNaN(event.wi)){
            debugPrintfEXT("whDotOut cosThetaT %f %f\n", whDotOut, cosThetaT);
        }
        record.pdf = dielectric_pdf(mat, event);
        record.f = dielectric_f(mat, event);
    }

    return record;
}



//Generate Code for Disney BSDF

//float disney_pdf(const RTMaterial mat, const SurfaceScatterEvent event){
//    return 0;
//}
//
//vec3 disney_f(const RTMaterial mat, const SurfaceScatterEvent event){
//    return vec3(0);
//}


vec3 clearcoat_f(const RTMaterial mat, const SurfaceScatterEvent event) {
    if (event.wi.z < 0.0 || event.wo.z < 0.0) {
        return vec3(0.0);
    }
    vec3 wh = normalize(event.wi + event.wo);
    float eta = 1.5;
    float R0 = pow((eta - 1.0) / (eta + 1.0), 2.0);
    float f = R0 + (1.0 - R0) * pow(1.0 - abs(dot(wh, event.wi)), 5.0);
    vec3 F = vec3(f);
    float roughness = mat.clearCoatGloss;
    float alphaG = (1.0 - roughness) * 0.1 + roughness * 0.001;
    float D = (alphaG * alphaG - 1.0) / (PI * 2.0 * log(alphaG) * (1.0 + (alphaG * alphaG - 1.0) * pow(wh.z, 2.0)));
    float G = smith_masking_g2(event.wi,0.25) * smith_masking_g2(event.wo,0.25);

    return F * D * G / (4.0 * abs(event.wi.z ));
}

float clearcoat_pdf(const RTMaterial mat, const SurfaceScatterEvent event) {
    if (event.wo.z < 0.0 || event.wi.z < 0.0) {
        return 0.0;
    }
    vec3 wh = normalize(event.wi + event.wo);
    float cosTheta2 = wh.z * wh.z;
    float roughness = mat.clearCoatGloss;
    float alphaG = (1.0 - roughness) * 0.1 + roughness * 0.001;
    float alphaG2 = alphaG * alphaG;

    float D = (alphaG2 - 1.0) / log(alphaG2) / PI / (1.0 + (alphaG2 - 1.0) * cosTheta2);
    return D * abs(wh.z) / (4.0 * abs(dot(wh, event.wi)));
}

BsdfSampleRecord clearcoat_sample(const RTMaterial mat, inout uvec4 seed, inout SurfaceScatterEvent event) {
    if (event.wo.z < 0.0) {
        return invalid_record();
    }
    BsdfSampleRecord result;
    float roughness = mat.clearCoatGloss;
    float alphaG = (1.0 - roughness) * 0.1 + roughness * 0.001;
    float alphaG2 = alphaG * alphaG;
    float cosTheta = sqrt((1.0 - pow(alphaG2, 1.0 - rand1(seed))) / (1.0 - alphaG2));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    float phi = rand1(seed) * PI * 2.0;
    vec3 wh = vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);

    event.wi = normalize(-event.wo + 2.0 * dot(event.wo, wh) * wh);
    result.sample_flags = RT_BSDF_LOBE_GLOSSY | RT_BSDF_LOBE_REFLECTION;
    result.pdf = clearcoat_pdf(mat, event);
    result.f = clearcoat_f(mat, event);
    return result;
}


#include "disney.glsl"



vec3 eval_bsdf(const RTMaterial mat, const SurfaceScatterEvent event) {
    uint bsdf_type = mat.bsdf_type;
    HANDLE_BSDF_F(DIFFUSE,diffuse)
HANDLE_BSDF_F(MIRROR,mirror)
HANDLE_BSDF_F(CONDUCTOR,conductor)
HANDLE_BSDF_F(PLASTIC,plastic)
HANDLE_BSDF_F(DIELECTRIC,dielectric)
HANDLE_BSDF_F(DISNEY,disney)
return vec3(0);
}

float pdf_bsdf(const RTMaterial mat, const SurfaceScatterEvent event) {
    uint bsdf_type = mat.bsdf_type;
    HANDLE_BSDF_PDF(DIFFUSE,diffuse)
HANDLE_BSDF_PDF(MIRROR,mirror)
HANDLE_BSDF_PDF(CONDUCTOR,conductor)
HANDLE_BSDF_PDF(PLASTIC,plastic)
HANDLE_BSDF_PDF(DIELECTRIC,dielectric)
HANDLE_BSDF_PDF(DISNEY,disney)
return 0;
}



BsdfSampleRecord sample_bsdf(const RTMaterial mat, inout SurfaceScatterEvent event, inout uvec4 seed) {
    uint bsdf_type = mat.bsdf_type;
    HANDLE_BSDF_SAMPLE(MIRROR,mirror)
HANDLE_BSDF_SAMPLE(DIFFUSE,diffuse)
HANDLE_BSDF_SAMPLE(CONDUCTOR,conductor)
HANDLE_BSDF_SAMPLE(PLASTIC,plastic)
HANDLE_BSDF_SAMPLE(DIELECTRIC,dielectric)
HANDLE_BSDF_SAMPLE(DISNEY,disney)
BsdfSampleRecord record;
return record;
}

#endif