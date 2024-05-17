vec3 sample_specify_light(const uint light_idx, inout SurfaceScatterEvent event, bool enable_sample_light, bool enable_sample_bsdf,inout uvec4 seed){

    vec3 result = vec3(0);
    
    vec3 light_sample_rand = rand3(seed);

    if(light_idx>=48){
        debugPrintfEXT("light_idx %d\n", light_idx);
    }
   // return result;
    const RTLight light = lights[light_idx];


    LightSample light_sample = sample_li(light, event, light_sample_rand);


    if (enable_sample_light)
    {
        if (!isBlack(light_sample.indensity) && light_sample.pdf != 0){

            //  return light_sample.indensity / light_sample.pdf;
            any_hit_payload.hit = 1;

         //   return vec3(0);
            traceRayEXT(tlas,
            gl_RayFlagsTerminateOnFirstHitEXT |
            gl_RayFlagsSkipClosestHitShaderEXT,
            0xFF, 1, 0, 1, event.p, EPS, light_sample.wi, light_sample.dist - EPS, 1);
            bool  visible = any_hit_payload.hit == 0;
            if (visible){
                uint material_idx = event.material_idx;
                event.wo = to_local(event.frame, light_sample.wi);
                float bsdf_pdf = pdf_bsdf(materials.m[material_idx], event);
                float light_mis_weight =enable_sample_bsdf? power_heuristic(light_sample.pdf, bsdf_pdf):1;
                vec3 bsdf =eval_bsdf(materials.m[material_idx], event);
                result += light_sample.indensity  * bsdf * light_mis_weight / light_sample.pdf;
            }
        }
    }

    if (enable_sample_bsdf)
    {
        BsdfSampleRecord record = sample_bsdf(materials.m[event.material_idx], event, rand2(seed));
        vec3 f = record.f;
        float bsdf_pdf = record.pdf;

        if (f != vec3(0) && bsdf_pdf != 0){


            vec3 world_wi = to_world(event.frame, event.wo);
            //trace ray 
            hitPayload.prim_idx = -1;
            traceRayEXT(tlas,
            gl_RayFlagsOpaqueEXT,
            0xFF, 0, 0, 0, event.p, EPS, world_wi, 10000, 0);

            bool same_light = hitPayload.prim_idx == light.prim_idx;

            //return visualize_normal(world_wi);

            // debugPrintfEXT("prim_idx %d %d\n", hitPayload.prim_idx, light.prim_idx);
            if (same_light){

                uint material_idx = event.material_idx;

                float light_pdf = 1.f/get_primitive_area(light.prim_idx) *  length(hitPayload.p-event.p) * length(hitPayload.p-event.p) / abs(dot(hitPayload.n_g, world_wi));

                float bsdf_mis_weight = enable_sample_light?power_heuristic(bsdf_pdf, light_pdf):1;

                result += f * eval_light(light, hitPayload.p, hitPayload.n_g, world_wi) * bsdf_mis_weight / bsdf_pdf;

            }

        }
    }

    return result;
}


vec3   uniform_sample_one_light(inout uvec4 seed, inout SurfaceScatterEvent event, const uint light_num,bool enable_sample_light, bool enable_sample_bsdf){

    float light_choose_rand = rand1(seed);

    uint light_idx = uint(light_choose_rand * light_num);
    float light_choose_pdf = 1.0 / light_num;

    vec3 result = sample_specify_light(light_idx, event,  enable_sample_light, enable_sample_bsdf,seed) / light_choose_pdf;

    return result;

}

