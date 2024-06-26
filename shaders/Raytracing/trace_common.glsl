vec3 sample_specify_light(const uint light_idx, inout SurfaceScatterEvent event, bool enable_sample_light, bool enable_sample_bsdf, inout uvec4 seed){

    vec3 result = vec3(0);

    vec3 light_sample_rand = rand3(seed);

    if (is_specular_material(materials.m[event.material_idx])){
        return vec3(0, 0, 0);
    }
    const RTLight light = lights[light_idx];


    LightSample light_sample = sample_li(light, event, light_sample_rand);


    if (enable_sample_light)
    {
        if (!isBlack(light_sample.indensity) && light_sample.pdf != 0){

            any_hit_payload.hit = 1;

            traceRayEXT(tlas,
            gl_RayFlagsTerminateOnFirstHitEXT |
            gl_RayFlagsSkipClosestHitShaderEXT,
            0xFF, 1, 0, 1, event.p, EPS, light_sample.wi, light_sample.dist - EPS, 1);
            bool  visible = any_hit_payload.hit == 0;
            if (visible){
                uint material_idx = event.material_idx;
                event.wi = to_local(event.frame, light_sample.wi);
                float bsdf_pdf = pdf_bsdf(materials.m[material_idx], event);
                float light_mis_weight =enable_sample_bsdf? power_heuristic(light_sample.pdf, bsdf_pdf):1;
                vec3 bsdf =eval_bsdf(materials.m[material_idx], event);
                result += light_sample.indensity  * bsdf * light_mis_weight / light_sample.pdf;
                if (luminance(result) > 1000){
                    //    debugPrintfEXT("light_sample.indensity %f %f %f\n", light_sample.indensity.x, light_sample.indensity.y, light_sample.indensity.z);
                    //  debugPrintfEXT("bsdf %f %f %f\n", bsdf.x, bsdf.y, bsdf.z);
                    //                    debugPrintfEXT("light_mis_weight %f\n", light_mis_weight);
                    //                    debugPrintfEXT("light_sample.pdf %f\n", light_sample.pdf);
                    //                    debugPrintfEXT("bsdf_pdf %f\n", bsdf_pdf);
                    //                    debugPrintfEXT("result %f %f %f\n", result.x, result.y, result.z);
                }
            }
        }
    }

    if (enable_sample_bsdf)
    {
        BsdfSampleRecord record = sample_bsdf(materials.m[event.material_idx], event, rand2(seed));
        vec3 f = record.f;
        float bsdf_pdf = record.pdf;
        if (f != vec3(0) && bsdf_pdf != 0){

            vec3 world_wi = to_world(event.frame, event.wi);
            //trace ray 

            hitPayload.prim_idx = -1;
            traceRayEXT(tlas,
            gl_RayFlagsOpaqueEXT,
            0xFF, 0, 0, 0, event.p + EPS * world_wi, 0, world_wi, 10000, 0);

            bool same_light = light_sample.is_infinite? hitPayload.prim_idx == -1: hitPayload.prim_idx == light.prim_idx;

            if (same_light){
                uint material_idx = event.material_idx;

                float light_pdf = eval_light_pdf(light, event.p, hitPayload.p, hitPayload.n_g, world_wi);

                float bsdf_mis_weight = enable_sample_light?power_heuristic(bsdf_pdf, light_pdf):1;

                vec3 sample_bsdf_result = f * eval_light(light, hitPayload.p, hitPayload.n_g, world_wi) * bsdf_mis_weight / bsdf_pdf;
                if (isnan(sample_bsdf_result.x) || isnan(sample_bsdf_result.y) || isnan(sample_bsdf_result.z)){
                    debugPrintfEXT("f %f %f %f\n", f.x, f.y, f.z);
                    debugPrintfEXT("eval_light %f %f %f\n", eval_light(light, hitPayload.p, hitPayload.n_g, world_wi).x, eval_light(light, hitPayload.p, hitPayload.n_g, world_wi).y, eval_light(light, hitPayload.p, hitPayload.n_g, world_wi).z);
                    debugPrintfEXT("bsdf_mis_weight %f\n", bsdf_mis_weight);
                    debugPrintfEXT("bsdf_pdf %f light_pdf %f\n", bsdf_pdf, light_pdf);
                    debugPrintfEXT("sample_bsdf_result %f %f %f\n", sample_bsdf_result.x, sample_bsdf_result.y, sample_bsdf_result.z);
                }
                result += sample_bsdf_result;

            }

        }
    }

    return result;
}

bool is_delta_material(const uint material_idx){
    return materials.m[material_idx].roughness < EPS;
}


vec3   uniform_sample_one_light(inout uvec4 seed, inout SurfaceScatterEvent event, const uint light_num, bool enable_sample_light, bool enable_sample_bsdf){

    //    bool is_delta_material = is_delta_material(event.material_idx);
    //    if (is_delta_material){
    //        return vec3(0);
    //    }
    float light_choose_rand = rand1(seed);

    uint light_idx = uint(light_choose_rand * light_num);
    float light_choose_pdf = 1.0 / light_num;

    vec3 result = sample_specify_light(light_idx, event, enable_sample_light, enable_sample_bsdf, seed) / light_choose_pdf;

    return result;

}

