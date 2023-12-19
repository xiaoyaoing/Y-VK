#ifndef COMMONS_GLSL
#define COMMONS_GLSL

#include "commons.h"
#include "util.glsl"


vec3 diffuse_f(const RTMaterial mat,const SurfaceScatterEvent event){
    return mat.albedo * INV_PI * max(0,get_cos_theta(event.wi));
}

vec3 diffuse_sample(const RTMaterial mat,const vec2 rand ,inout SurfaceScatterEvent event,out float pdf){
    event.wi = square_to_cosine_hemisphere(rand);
    pdf = square_to_cosine_hemisphere_pdf(event.wi);
    return diffuse_f(mat,event);
}

float diffuse_pdf(const RTMaterial mat,const SurfaceScatterEvent event){
    return square_to_cosine_hemisphere_pdf(event.wi);
}


//BsdfSampleRecord diffuse_sample(const RTMaterial mat,const vec3 wo,const vec3 n_s){
//    BsdfSampleRecord bsdf_sample;
//    bsdf_sample.wi = sample_hemisphere_cosine(n_s);
//    bsdf_sample.f = diffuse_f(mat,bsdf_sample.wi,n_s);
//    bsdf_sample.pdf = diffuse_pdf(mat,wo,bsdf_sample.wi,n_s);
//    return bsdf_sample;
//}

//vec3 eval_bsdf(const RTMaterial mat,const vec3 wo,const vec3 wi,const vec3 n_s,out float pdf_w) {
//    vec3 f;
//    f = diffuse_f(mat,wi,n_s);
//    return f;
////    pdf_w = diffuse_pdf(mat,wo,wi,n_s);
////    return f;
//}

vec3 eval_bsdf(const RTMaterial mat,const SurfaceScatterEvent event) {
    vec3 f;
    f = diffuse_f(mat,event);
    return f;
}

float pdf_bsdf(const RTMaterial mat,const SurfaceScatterEvent event) {
    return diffuse_pdf(mat,event);
}

vec3  sample_bsdf(const RTMaterial mat,inout SurfaceScatterEvent event,const vec2 rand,out float pdf){
    return diffuse_sample(mat,rand,event,pdf);
}

#endif