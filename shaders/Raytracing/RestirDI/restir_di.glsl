#include "di_commons.h"

uvec4 seed ;

SurfaceScatterEvent g_event;

layout(buffer_reference, std430, buffer_reference_align = 4) readonly buffer RestirReservoir_{ RestirReservoir r[]; };


RestirReservoir_ temporal_reservoir = RestirReservoir_(scene_desc.restir_temporal_reservoir_addr);
RestirReservoir_ spatial_reservoir = RestirReservoir_(scene_desc.restir_spatial_reservoir_addr);
RestirReservoir_ pass_reservoir = RestirReservoir_(scene_desc.restir_pass_reservoir_addr);

RestirReservoir init_restir_reservoir(){
    RestirReservoir reservoir;
    reservoir.w_sum = 0;
    reservoir.m = 0;
    reservoir.w_sum = 0;
    
    
    return reservoir;
}

//this is glsl file not cpp
void update_restir_reservoir(inout RestirReservoir  r_new, RestirData s, float w_i){
    r_new.w_sum += w_i;
    r_new.m++;
    if (rand1(seed) < w_i / r_new.w_sum) {
        r_new.s = s;
    }
}




vec3  restir_sample_light(inout SurfaceScatterEvent event, in uint light_idx){
    vec3 result = vec3(0);

    float light_choose_rand = rand1(seed);
    
    vec3 light_sample_rand = rand3(seed);
    const RTLight light = lights[light_idx];
    
    LightSample light_sample = sample_li(light, event, light_sample_rand);
    
    uint material_idx = event.material_idx;
    event.wo = to_local(event.frame, light_sample.wi);
    float bsdf_pdf = pdf_bsdf(materials.m[material_idx], event);
    vec3 bsdf =eval_bsdf(materials.m[material_idx], event);
    
    if (light_sample.pdf >0) {
        result = light_sample.indensity * bsdf  / light_sample.pdf;
    }
//    debugPrintfEXT("result %f %f %f\n",result.x,result.y,result.z);
    return result;
    
}

float calc_p_hat(const RestirReservoir r) {
    return length(restir_sample_light(g_event,r.s.light_idx));
}

void combine_reservoir(inout RestirReservoir r1, const RestirReservoir r2) {
    float fac = r2.W * r2.m;
    if (fac > 0) {
        fac *= calc_p_hat(r2);
    }
    update_restir_reservoir(r1, r2.s, fac);
}

