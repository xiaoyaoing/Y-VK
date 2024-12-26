#ifndef DISNEY_BSDF_FUNCTIONS_GLSL
#define DISNEY_BSDF_FUNCTIONS_GLSL

const float PI = 3.141592653589793;

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

// Disney's diffuse term
vec3 disney_diffuse(const vec3 baseColor, float roughness, float NoL, float NoV, float VoH) {
    float fd90 = 0.5 + 2 * roughness * NoL * NoL;
    float f0 = 1 + (fd90 - 1) * pow(1 - VoH, 5);
    float lightScatter = (1 + (fd90 - 1) * pow(1 - NoL, 5)) * (1 + (fd90 - 1) * pow(1 - NoV, 5));
    float f = f0 * lightScatter;
    return baseColor * (1 - f) / PI;
}

// Disney's sheen term
vec3 disney_sheen(const vec3 baseColor, float sheenTint, float NoL, float NoV, float VoH) {
    float sheenLerp = (1 - sheenTint) + sheenTint * (dot(baseColor, vec3(0.3333)) > 0.5 ? 1 : 0);
    float sheen = pow(saturate(1 - NoL), 5) * pow(saturate(1 - NoV), 5) * sheenLerp;
    return baseColor * sheen;
}


vec3 disney_f(const DisneyBSDF disneyBXDF, const vec3 outgoing, const vec3 incoming) {
    vec3 halfway = normalize(incoming + outgoing);
    float NoL = saturate(dot(incoming, halfway));
    float NoV = saturate(dot(outgoing, halfway));
    float VoH = saturate(dot(outgoing, halfway));
    float LoH = saturate(dot(incoming, halfway));

    // Diffuse component
    vec3 diffuse = disney_diffuse(disneyBXDF.diffuse.baseColor, disneyBXDF.diffuse.roughness, NoL, NoV, VoH);

    // Sheen component
    vec3 sheen = disney_sheen(disneyBXDF.sheen.baseColor, disneyBXDF.sheen.sheenTint, NoL, NoV, VoH);

    // Metal component (simplified version, you need to implement the full Disney metal model)
    vec3 metal = vec3(0); // Placeholder for metal component

    // Clear coat component (simplified version, you need to implement the full Disney clear coat model)
    vec3 clearCoat = vec3(0); // Placeholder for clear coat component

    // Glass component (simplified version, you need to implement the full Disney glass model)
    vec3 glass = vec3(0); // Placeholder for glass component

    // Combine all components
    return diffuse + sheen + metal + clearCoat + glass;
}




#endif // DISNEY_BSDF_FUNCTIONS_GLSL