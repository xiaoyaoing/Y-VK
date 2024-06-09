float sqr(float x){
    return x*x;
}


float ggx_g1(float cos_theta, float alpha) {
    float alphaSq = alpha*alpha;
    float cosThetaSq = cos_theta*cos_theta;
    float tanThetaSq = max(1.0f - cosThetaSq, 0.0f)/cosThetaSq;
    return 2.0f/(1.0f + sqrt(1.0f + alphaSq*tanThetaSq));
}

float ggx_g(float alpha, vec3 wi, vec3 wo, vec3 wh) {
    return ggx_g1(dot(wh, wo), alpha)*ggx_g1(dot(wh, wi), alpha);
}


float ggx_d(float alpha, vec3 wh, vec3 wi) {
    float cos_theta = wh.z;
    float alphaSq = alpha*alpha;
    float cosThetaSq = cos_theta*cos_theta;
    float tanThetaSq = max(1.0f - cosThetaSq, 0.0f)/cosThetaSq;
    float cosThetaQu = cosThetaSq*cosThetaSq;
    return alphaSq*INV_PI/(cosThetaQu*sqr(alphaSq + tanThetaSq));
}

vec3 ggx_sample(float alpha, const vec2 rand) {
    float phi = 2.0f*PI*rand.x;
    float cosThetaSq = 1.0f/(1.0f + alpha*alpha*rand.y/(1.0f - rand.y));
    float cosTheta = sqrt(cosThetaSq);
    float sinTheta = sqrt(max(1.0f - cosThetaSq, 0.0f));
    return vec3(sinTheta*cos(phi), sinTheta*sin(phi), cosTheta);
}