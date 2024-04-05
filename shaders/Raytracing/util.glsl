#ifndef UTILS_DEVICE
#define UTILS_DEVICE


#define PI 3.14159265359
#define PI2 6.28318530718
#define INV_PI (1. / PI)
#define INF 1e10
#define EPS 0.001
#define SHADOW_EPS 2 / 65536.
#define sqrt2 1.4v1421356237309504880
#define EPS 0.001


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

struct Frame{
    vec3 tangent;
    vec3 bitTangent;
    vec3  n;
};

vec3 to_local(const Frame frame, const vec3 v){
    return vec3(dot(v, frame.tangent), dot(v, frame.bitTangent), dot(v, frame.n));
}

vec3 to_world(const Frame frame, const vec3 v){
    return frame.tangent * v.x + frame.bitTangent * v.y + frame.n * v.z;
}

Frame make_frame(const vec3 n){
    Frame frame;
    frame.n = n;
    if (abs(n.z) < 0.999) {
        frame.tangent = normalize(cross(n, vec3(0, 0, 1)));
    } else {
        frame.tangent = normalize(cross(n, vec3(0, 1, 0)));
    }
    frame.bitTangent = normalize(cross(n, frame.tangent));
    return frame;
}

struct SurfaceScatterEvent{
    vec3 wo;
    vec3 wi;
    vec3 p;
    vec2 uv;
    Frame frame;

    uint material_idx;
    uint triangle_idx;
};



SurfaceScatterEvent make_suface_scatter_event(const HitPayload hit_pay_load, const vec3 wo){
    SurfaceScatterEvent event;

    event.frame = make_frame(hit_pay_load.n_s);
    event.wo = to_local(event.frame, wo);
    event.p = hit_pay_load.p;
    event.material_idx = hit_pay_load.material_idx;
    event.uv = hit_pay_load.uv;
    return event;
}

float get_cos_theta(const vec3 v){
    return v.z;
}


vec3 square_to_uniform_hemisphere(const vec2 rand) {
    float z = 1 - 2 * rand[0];
    float r = sqrt(max(0.f, 1.f - z * z));
    float phi = 2 * PI * rand[1];
    return vec3(r * cos(phi), r * sin(phi), z);
}

float square_to_uniform_hemisphere_pdf(const vec3 v) {
    return v[2] >= 0 ? 0.5f * INV_PI : .0f;
}

vec3 square_to_cosine_hemisphere(const vec2 rand) {
    float z = sqrt(1 - rand.x);
    float phi = rand.y * 2 * PI;
    return vec3(sqrt(rand.x) * cos(phi), sqrt(rand.x) * sin(phi), z);
}

float square_to_cosine_hemisphere_pdf(const vec3 v) {
    return v[2] >= 0 ? v.z * INV_PI : .0f;
}

float power_heuristic(float a, float b) {
    return (a * a) / (a * a + b * b);
}



#endif 