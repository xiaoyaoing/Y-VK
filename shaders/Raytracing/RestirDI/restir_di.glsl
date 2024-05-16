#include "di_commons.h"

uvec4 seed ;

SurfaceScatterEvent g_event;

layout(buffer_reference, std430, buffer_reference_align = 4) readonly buffer RestirReservoir_{ RestirReservoir r[]; };
layout(buffer_reference, std430, buffer_reference_align = 4) readonly buffer GBuffer_{ GBuffer r[]; };


RestirReservoir_ temporal_reservoirs = RestirReservoir_(scene_desc.restir_temporal_reservoir_addr);
RestirReservoir_ spatial_reservoirs = RestirReservoir_(scene_desc.restir_spatial_reservoir_addr);
RestirReservoir_ pass_reservoir = RestirReservoir_(scene_desc.restir_pass_reservoir_addr);
GBuffer_ gbuffer = GBuffer_(scene_desc.gbuffer_addr);

uint pixel_idx = gl_LaunchIDEXT.x * gl_LaunchSizeEXT.y + gl_LaunchIDEXT.y;
vec3 gBuffer_normal;
vec3 gBuffer_position;
uint gBuffer_material_idx;
vec2 gBuffer_uv; 

void load_gbuffer(){
    gBuffer_normal = gbuffer.r[pixel_idx].normal;
    gBuffer_position = gbuffer.r[pixel_idx].position;
    gBuffer_material_idx = gbuffer.r[pixel_idx].material_idx;
    gBuffer_uv = gbuffer.r[pixel_idx].uv;
}


RestirReservoir init_restir_reservoir(){
    RestirReservoir reservoir;
    reservoir.W = 0;
    reservoir.m = 0;
    reservoir.w_sum = 0;
    reservoir.s.light_idx = -1;
    
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
    
    if(light_idx>48){
        return result;
    }
    
    const RTLight light = lights[light_idx];

    LightSample light_sample = sample_li(light, event, light_sample_rand);
    
    uint material_idx = event.material_idx;
    event.wo = to_local(event.frame, light_sample.wi);

    vec3 bsdf =eval_bsdf(materials.m[material_idx], event);

    if (light_sample.pdf >0) {
        result = light_sample.indensity * bsdf  / light_sample.pdf;
    }
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

