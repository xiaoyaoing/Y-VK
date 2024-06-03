#include "di_commons.h"

uvec4 seed;

SurfaceScatterEvent g_event;

layout(buffer_reference, std430, buffer_reference_align = 4) readonly buffer RestirReservoir_{ RestirReservoir r[]; };
layout(buffer_reference, std430, buffer_reference_align = 4) readonly buffer GBuffer_{ GBuffer r[]; };


RestirReservoir_ temporal_reservoirs = RestirReservoir_(scene_desc.restir_temporal_reservoir_addr);
RestirReservoir_ spatial_reservoirs = RestirReservoir_(scene_desc.restir_spatial_reservoir_addr);
RestirReservoir_ pass_reservoir = RestirReservoir_(scene_desc.restir_pass_reservoir_addr);
GBuffer_ gbuffer = GBuffer_(scene_desc.gbuffer_addr);

uint    pixel_idx =  gl_LaunchIDEXT.y * gl_LaunchSizeEXT.x + gl_LaunchIDEXT.x;
vec3 gBuffer_normal;
vec3 gBuffer_position;
uint gBuffer_material_idx;
vec2 gBuffer_uv;


void load_gbuffer(){
    gBuffer_normal = gbuffer.r[pixel_idx].normal;
    gBuffer_position = gbuffer.r[pixel_idx].position;
    gBuffer_material_idx = gbuffer.r[pixel_idx].material_idx;
    gBuffer_uv = gbuffer.r[pixel_idx].uv;
    vec3 origin = (scene_ubo.viewInverse * vec4(0, 0, 0, 1)).xyz;
    vec3 dir = normalize(gBuffer_position - origin);
    g_event =make_surface_scatter_event(-dir, gBuffer_normal, gBuffer_position, gBuffer_uv, gBuffer_material_idx);
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





vec3 restir_sample_light(inout SurfaceScatterEvent event, const vec4 rand, const uint light_num, out uint light_idx, out LightSample light_sample){
    vec3 result = vec3(0);

    float light_choose_rand =rand.x;

    light_idx = uint(light_choose_rand * light_num);

    vec3 light_sample_rand = rand.yzw;

    const RTLight light = lights[light_idx];

    light_sample = sample_li(light, event, light_sample_rand);

    uint material_idx = event.material_idx;
    event.wi = to_local(event.frame, light_sample.wi);

    vec3 bsdf =eval_bsdf(materials.m[material_idx], event);

    if (light_sample.pdf >0) {
        result = light_sample.indensity * bsdf  / light_sample.pdf;
    }
    //result = vec3(1, 0, 0);

    return result;
}

vec3 restir_sample_light(in SurfaceScatterEvent event, const vec4 rand, const uint light_num){
    vec3 result = vec3(0);

    float light_choose_rand = rand.x;

    uint light_idx = uint(light_choose_rand * light_num);

    vec3 light_sample_rand = rand.yzw;

    const RTLight light = lights[light_idx];

    LightSample light_sample = sample_li(light, event, light_sample_rand);

    uint material_idx = event.material_idx;
    event.wi = to_local(event.frame, light_sample.wi);

    vec3 bsdf =eval_bsdf(materials.m[material_idx], event);

    if (light_sample.pdf >0) {
        result = light_sample.indensity * bsdf  / light_sample.pdf;
    }
    //   result = vec3(1, 0, 0);
    return result;
}

vec3 restir_sample_light_with_vis(in SurfaceScatterEvent event, const vec4 rand, const uint light_num){
    vec3 result = vec3(0);

    float light_choose_rand = rand.x;

    uint light_idx = uint(light_choose_rand * light_num);

    vec3 light_sample_rand = rand.yzw;

    const RTLight light = lights[light_idx];

    LightSample light_sample = sample_li(light, event, light_sample_rand);

    if (light_sample.pdf ==0 || isBlack(light_sample.indensity)){
        return vec3(0);
    }

    bool visible = true;
    if (light_sample.pdf >0) {
        traceRayEXT(tlas,
        gl_RayFlagsTerminateOnFirstHitEXT |
        gl_RayFlagsSkipClosestHitShaderEXT,
        0xFF, 1, 0, 1, event.p, EPS, light_sample.wi, light_sample.dist - EPS, 1);
        visible = any_hit_payload.hit == 0;
    }
    if (!visible){
        return vec3(0);
    }

    uint material_idx = event.material_idx;
    event.wi = to_local(event.frame, light_sample.wi);

    vec3 bsdf =eval_bsdf(materials.m[material_idx], event);

    if (light_sample.pdf >0) {
        result = light_sample.indensity * bsdf  / light_sample.pdf;
    }
    return result;

}


vec3 calc_L(const RestirReservoir r){
    uvec4 seed = r.s.seed;
    return restir_sample_light(g_event, rand4(seed), 48);

    vec3 result = vec3(0);


    uint light_idx = r.s.light_idx;
    uint triangle_idx = r.s.triangle_idx;
    uvec4 r_seed = r.s.seed;

    const RTLight light = lights[light_idx];



    LightSample   light_sample = sample_li_area_light_with_idx(light, g_event, rand3(r_seed), triangle_idx);

    uint material_idx = g_event.material_idx;
    g_event.wi = to_local(g_event.frame, light_sample.wi);

    vec3 bsdf =eval_bsdf(materials.m[material_idx], g_event);

    if (light_sample.pdf >0) {
        result = light_sample.indensity * bsdf  / light_sample.pdf;
    }
    return result;
}

vec3 calc_L_vis(const RestirReservoir r){
    uvec4 r_seed = r.s.seed;

    return restir_sample_light_with_vis(g_event, rand4(r_seed), 48);
    vec3 result = vec3(0);


    uint light_idx = r.s.light_idx;
    uint triangle_idx = r.s.triangle_idx;

    const RTLight light = lights[light_idx];

    LightSample   light_sample = sample_li_area_light_with_idx(light, g_event, rand3(r_seed), triangle_idx);
    if (!isBlack(light_sample.indensity) && light_sample.pdf != 0){

        traceRayEXT(tlas,
        gl_RayFlagsTerminateOnFirstHitEXT |
        gl_RayFlagsSkipClosestHitShaderEXT,
        0xFF, 1, 0, 1, g_event.p, EPS, light_sample.wi, light_sample.dist - EPS, 1);
        bool  visible = any_hit_payload.hit == 0;
        //  visible = true;
        if (!visible){
            return vec3(0);
        }

        uint material_idx = g_event.material_idx;
        // debugPrintfEXT("material_idx %d\n", material_idx);
        g_event.wi = to_local(g_event.frame, light_sample.wi);

        vec3 bsdf =eval_bsdf(materials.m[material_idx], g_event);

        result = light_sample.indensity * bsdf  / light_sample.pdf;
    }
    return result;
}



float calc_p_hat(const RestirReservoir r) {
    return length(calc_L(r));
}

void combine_reservoir(inout RestirReservoir r1, const RestirReservoir r2) {
    float fac = r2.W * r2.m;
    if (fac > 0) {
        fac *= calc_p_hat(r2);
    }
    update_restir_reservoir(r1, r2.s, fac);
}

void printf_restir_reservoir(const RestirReservoir r){
    debugPrintfEXT("all message in one line %f %d %f %d %d %d %d %d %d %f  \n", r.W, r.m, r.w_sum, r.s.light_idx, r.s.triangle_idx, r.s.seed.x, r.s.seed.y, r.s.seed.z, r.s.seed.w, calc_p_hat(r));
}




