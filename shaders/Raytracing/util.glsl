#ifndef UTILS_DEVICE
#define UTILS_DEVICE


#define PI 3.14159265359
#define PI2 6.28318530718
#define INV_PI (1. / PI)
#define INF 1e10
#define EPS 0.001
#define SHADOW_EPS 2 / 65536.
#define sqrt2 1.41421356237309504880
#define EPS 0.001


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


#endif 