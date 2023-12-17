#ifndef UTILS_DEVICE
#define UTILS_DEVICE
struct HitPayload{
    vec3 n_g;
    vec3 n_s;
    vec3 p;
    vec2 uv;
    uint material_idx;
    uint triangle_idx;
};

struct AnyHitPayload {
    int hit;
};

#define EPS 0.001

#endif 