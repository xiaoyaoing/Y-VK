//#ifndef COMMONS_GLSL
//#define COMMONS_GLSL

#include "commons.h"
#include "util.glsl"

vec3 diffuse_f(const RTMaterial mat,const vec3 wi,const vec3 n_s){
    return mat.albedo * INV_PI * (dot(wi,n_s));
}

float diffuse_pdf(const RTMaterial mat,const vec3 wo,const vec3 wi,const vec3 n_s){
    //sample hem 
    return max(dot(wi,n_s),0.f);
}

vec3 eval_bsdf(const RTMaterial mat,const vec3 wo,const vec3 wi,const vec3 n_s,out float pdf_w) {
    vec3 f;
    f = diffuse_f(mat,wi,n_s);
    pdf_w = diffuse_pdf(mat,wo,wi,n_s);
    return f;
}
//#endif