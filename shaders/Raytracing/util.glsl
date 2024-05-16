#ifndef UTILS_DEVICE
#define UTILS_DEVICE




struct HitPayload{
    vec3 n_g;
    vec3 n_s;
    vec3 p;
    vec2 uv;
    uint material_idx;
    uint triangle_idx;
    uint prim_idx;
};

struct AnyHitPayload {
    int hit;
};

struct BsdfSampleRecord{
    vec3 f;
    float pdf;
    uint sample_flags;
};




struct SurfaceScatterEvent{
    vec3 wo;
    vec3 wi;
    vec3 p;
    vec2 uv;
    Frame frame;

    uint material_idx;
    uint triangle_idx;
};

uint jenkinsHash(uint a)
{
    a = (a + 0x7ed55d16) + (a << 12);
    a = (a ^ 0xc761c23cu) ^ (a >> 19);
    a = (a + 0x165667b1) + (a << 5);
    a = (a + 0xd3a2646c) ^ (a << 9);
    a = (a + 0xfd7046c5) + (a << 3);
    a = (a ^ 0xb55a4f09u) ^ (a >> 16);
    return a;
}

vec3 pseudocolor(uint value)
{
    uint h = jenkinsHash(value);
    return (uvec3(h, h >> 8, h >> 16) & 0xffu) / 255.f;
}



SurfaceScatterEvent make_surface_scatter_event(HitPayload hit_pay_load, const vec3 wo){
    SurfaceScatterEvent event;


    //Todo Handle two side bsdf 
    if (dot(wo, hit_pay_load.n_s) < 0){
        hit_pay_load.n_s = -hit_pay_load.n_s;
    }

    event.frame = make_frame(hit_pay_load.n_s);
    event.wi = to_local(event.frame, wo);
    event.p = hit_pay_load.p;
    event.material_idx = hit_pay_load.material_idx;
    event.uv = hit_pay_load.uv;
    return event;
}

float get_cos_theta(const vec3 v){
    return v.z;
}

float sqr(float x){
    return x*x;
}



float power_heuristic(float a, float b) {
    return (a * a) / (a * a + b * b);
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


float dielectricReflectance(float eta, float cosThetaI, float cosThetaT) {
    if (cosThetaI < 0.0) {
        eta       = 1.0 / eta;
        cosThetaI = -cosThetaI;
    }
    float sinThetaTSq = eta * eta * (1.0f - cosThetaI * cosThetaI);
    if (sinThetaTSq > 1.0) {
        cosThetaT = 0.0;
        return 1.0;
    }
    cosThetaT = sqrt(max(1.0 - sinThetaTSq, 0.0));

    float Rs = (eta * cosThetaI - cosThetaT) / (eta * cosThetaI + cosThetaT);
    float Rp = (eta * cosThetaT - cosThetaI) / (eta * cosThetaT + cosThetaI);

    return (Rs * Rs + Rp * Rp) * 0.5;
}

float dielectricReflectance(float eta, float cosThetaI) {
    float cosThetaT;
    return dielectricReflectance(eta, cosThetaI, cosThetaT);
}

float SchlickApproxFresnel(float eta, float cosTheta) {
    float r0 = (eta * eta - 2 * eta + 1) / (eta * eta + 2 * eta + 1);
    return r0 + (1 - r0) * pow(1 - cosTheta, 5);
}

float conductorReflectance(float eta, float k, float cosThetaI) {
    float cosThetaISq = cosThetaI * cosThetaI;
    float sinThetaISq = max(1.0f - cosThetaISq, 0.0f);
    float sinThetaIQu = sinThetaISq * sinThetaISq;

    float innerTerm  = eta * eta - k * k - sinThetaISq;
    float aSqPlusBSq = sqrt(max(innerTerm * innerTerm + 4.0f * eta * eta * k * k, 0.0f));
    float a          = sqrt(max((aSqPlusBSq + innerTerm) * 0.5f, 0.0f));

    float Rs = ((aSqPlusBSq + cosThetaISq) - (2.0f * a * cosThetaI)) /
    ((aSqPlusBSq + cosThetaISq) + (2.0f * a * cosThetaI));
    float Rp = ((cosThetaISq * aSqPlusBSq + sinThetaIQu) - (2.0f * a * cosThetaI * sinThetaISq)) /
    ((cosThetaISq * aSqPlusBSq + sinThetaIQu) + (2.0f * a * cosThetaI * sinThetaISq));

    return 0.5f * (Rs + Rs * Rp);
}

vec3 conductorReflectanceVec3(const vec3 eta, const vec3 k, float cosThetaI) {
    return vec3(
    conductorReflectance(eta.x, k.x, cosThetaI),
    conductorReflectance(eta.y, k.y, cosThetaI),
    conductorReflectance(eta.z, k.z, cosThetaI));
}

float diffuseReflectance(float eta, float sampleCount) {
    double diffuseFresnel = 0.0;
    float  fb             = dielectricReflectance(eta, 0.0f);
    for (int i = 1; i <= sampleCount; ++i) {
        float cosThetaSq = float(i) / sampleCount;
        float fa         = dielectricReflectance(eta, min(sqrt(cosThetaSq), 1.0f));
        diffuseFresnel += double(fa + fb) * (0.5 / sampleCount);
        fb = fa;
    }
    return float(diffuseFresnel);
}

bool sample_microfacet_reflection(const vec3 wo, float roughness, const vec2 rand, out vec3 wi){
    return false;
}




#endif 