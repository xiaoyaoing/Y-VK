#ifndef DISNEY_BSDF_FUNCTIONS_GLSL
#define DISNEY_BSDF_FUNCTIONS_GLSL



// Smith masking-term for the GGX distribution
float smithG_G1(float roughness, float No) {
    return No / (No * (1 - roughness) + roughness);
}

// Smith masking-term for anisotropic GGX distribution
float smithG_G1_anisotropic(float roughnessX, float roughnessY, vec3 v) {
    float ax = roughnessX * roughnessX;
    float ay = roughnessY * roughnessY;
    float bx = v.x * v.x * ax;
    float by = v.y * v.y * ay;
    float cx = v.z * v.z;
    float g1 = cx / (cx + sqrt(bx + cx));
    float g2 = cx / (cx + sqrt(by + cx));
    return g1 * g2;
}

float FD(vec3 w, float FD90) {
    return 1.0 + (FD90 - 1.0) * pow(1.0 - abs(w.z), 5.0);
}

float FSS(vec3 w, float FSS90) {
    return 1.0 + (FSS90 - 1.0) * pow(1.0 - abs(w.z), 5.0);
}

//float luminance(vec3 color) {
//    return dot(color, vec3(0.2126, 0.7152, 0.0722));
//}

// Disney's diffuse term
vec3 disney_diffuse(const vec3 baseColor, float roughness, float subSurfaceFactor, vec3 wi, vec3 wo,float sheen_tint) {
    if (wi.z < 0.0 || wo.z < 0.0) {
        return vec3(0.0);
    }
    vec3 wh = normalize(wi + wo);

    vec3 baseColorPI = baseColor / PI;

    float FD90 = 0.5 + 2.0 * roughness * pow(abs(dot(wh, wi)), 2.0);
    vec3 baseDiffuse = baseColorPI * FD(wo, FD90) * FD(wi, FD90);

    float FSS90 = roughness * abs(dot(wh, wi));
    float scaleFactor = FSS(wi, FSS90) * FSS(wo, FSS90) * (1.0 / (abs(wi.z) + abs(wo.z)) - 0.5) + 0.5;
    vec3 subsurface = 1.25 * baseColorPI * scaleFactor;

    vec3 diffuse_result =  mix(baseDiffuse, subsurface, subSurfaceFactor);
    
    vec3 c_tint = luminance(baseColor) > 0.0 ? baseColor / luminance(baseColor) : vec3(1.0);
    vec3 sheen_result = vec3(1-sheen_tint) + sheen_tint * c_tint;
    sheen_result *= pow(1.0 - dot(wi, wh), 5.0);
    
    return diffuse_result + sheen_result;
}

//// Disney's sheen term
//vec3 disney_sheen(const vec3 baseColor, float sheenTint, float NoL, float NoV, float VoH) {
//
//}
//



BsdfSampleRecord disney_sample(const RTMaterial mat, inout uvec4 seed, inout SurfaceScatterEvent event){
    BsdfSampleRecord record;
    float roughness = get_roughness(mat, event.uv);
    float metallic =  get_metallness(mat, event.uv);
    vec3 outgoing = event.wo;

    if (outgoing.z <= 0) {
        return dielectric_sample(mat, seed, event);
    }

    // 根据权重选择不同的 BxDF 进行采样
    float weights[4];
    weights[0] = (1.0 - mat.metallic) * (1.0 - mat.specularTransmission);// Diffuse
    weights[1] = 1 - mat.specularTransmission * (1.0 - metallic);// Metal
    weights[2] = (1.0 - mat.metallic) * mat.specularTransmission;// Glass
    weights[3] = 0.25 * mat.clearCoat;// ClearCoat

    float totalWeight = weights[0] + weights[1] + weights[2] + weights[3];
    float r = rand1(seed) * totalWeight;
    int idx = 0;
    for (int i = 0; i < 4; ++i) {
        if (r < weights[i]) {
            idx = i;
            break;
        }
        r -= weights[i];
    }

    // 根据选择的 BxDF 进行采样
    if (idx == 0) {
        // Diffuse
        return diffuse_sample(mat, seed, event);
    } else if (idx == 1) {
        // Metal
        return conductor_albedo_sample(mat, seed, event);
    } else if (idx == 2) {
        // Glass
        return dielectric_sample(mat, seed, event);
    } else if (idx == 3) {
        return clearcoat_sample(mat, seed, event);
    }
    return record;
}


vec3 disney_f(const RTMaterial mat, const SurfaceScatterEvent event){
    float roughness = get_roughness(mat, event.uv);
    float metallic =  get_metallness(mat, event.uv);
    vec3 outgoing = event.wo;

    if (outgoing.z <= 0) {
        return dielectric_f(mat, event);
    }

    float glassWeight = (1.0 - mat.metallic) * mat.specularTransmission;
    float clearCoatWeight = 0.25 * mat.clearCoat;
    float sheenWeight = 0.5 * mat.sheen;
    float diffuseWeight = (1.0 - mat.metallic) * (1.0 - mat.specularTransmission);
    float metalWeight = 1 - mat.specularTransmission * (1.0 - metallic);

    vec3 albedo = get_albedo(mat, event.uv);
    vec3 diffuseResult = diffuseWeight>0?disney_diffuse(albedo, roughness, mat.subSurfaceFactor, event.wi, event.wo, mat.sheenTint):vec3(0);
    vec3 metalResult = metalWeight>0?conductor_albedo_f(mat, event):vec3(0);
    vec3 glassResult = glassWeight>0?dielectric_f(mat, event):vec3(0);
    vec3 clearCoatResult = clearCoatWeight>0?clearcoat_f(mat, event):vec3(0);
    
    if(hasNaN(diffuseResult)){
        debugPrintfEXT("diffuseResult %f %f %f\n", diffuseResult.x, diffuseResult.y, diffuseResult.z);
    }
    if(hasNaN(metalResult)){
        debugPrintfEXT("metalResult %f %f %f\n", metalResult.x, metalResult.y, metalResult.z);
    }
    if(hasNaN(glassResult)){
        debugPrintfEXT("glassResult %f %f %f\n", glassResult.x, glassResult.y, glassResult.z);
    }
    if(hasNaN(clearCoatResult)){
        debugPrintfEXT("clearCoatResult %f %f %f\n", clearCoatResult.x, clearCoatResult.y, clearCoatResult.z);
    }
    return diffuseResult * diffuseWeight + metalResult * metalWeight + glassResult * glassWeight + clearCoatResult * clearCoatWeight;

}

float disney_pdf(const RTMaterial mat, const SurfaceScatterEvent event){
    float roughness = get_roughness(mat, event.uv);
    float metallic =  get_metallness(mat, event.uv);
    vec3 outgoing = event.wo;

    if (outgoing.z <= 0) {
        return dielectric_pdf(mat, event);
    }

    float glassWeight = (1.0 - mat.metallic) * mat.specularTransmission;
    float clearCoatWeight = 0.25 * mat.clearCoat;
    float diffuseWeight = (1.0 - mat.metallic) * (1.0 - mat.specularTransmission);
    float metalWeight = 1 - mat.specularTransmission * (1.0 - metallic);

    float diffuseResult = diffuseWeight>0?diffuse_pdf(mat, event):0;
    float metalResult = metalWeight>0?conductor_albedo_pdf(mat, event):0;
    float glassResult = glassWeight>0?dielectric_pdf(mat, event):0;
    float clearCoatResult = clearCoatWeight>0?clearcoat_pdf(mat, event):0;
    
    float allWeight = diffuseWeight + metalWeight + glassWeight + clearCoatWeight;
    return (diffuseResult * diffuseWeight + metalResult * metalWeight + glassResult * glassWeight + clearCoatResult * clearCoatWeight) / allWeight;
}


#endif// DISNEY_BSDF_FUNCTIONS_GLSL