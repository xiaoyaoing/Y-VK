#ifndef SAMPLING
#define SAMPLING
#define PI 3.14159265359
#define PI2 6.28318530718
#define INV_PI 0.3183098861837697
#define INV_TWO_PI 0.1591549430918953
#define INF 1e10
#define EPS 1e-3f
#define SHADOW_EPS 2 / 65536.
#define sqrt2 1.4v1421356237309504880
struct Frame{
    vec3 tangent;
    vec3 bitTangent;
    vec3  n;
};


uvec4 init_rng(uvec2 pixel_coords, uvec2 resolution, uint frame_num) {
    return uvec4(pixel_coords.xy, frame_num, 0);
}



uvec4 init_rng(uvec2 pixel_coords) {
    return uvec4(pixel_coords.xy, 0, 0);
}

float uint_to_float(uint x) {
    return uintBitsToFloat(0x3f800000 | (x >> 9)) - 1.0f;
}

uvec4 pcg4d(uvec4 v) {
    v = v * 1664525u + 1013904223u;
    v.x += v.y * v.w;
    v.y += v.z * v.x;
    v.z += v.x * v.y;
    v.w += v.y * v.z;
    v = v ^ (v >> 16u);
    v.x += v.y * v.w;
    v.y += v.z * v.x;
    v.z += v.x * v.y;
    v.w += v.y * v.z;
    return v;
}

float rand1(inout uvec4 rng_state) {
    rng_state.w++;
    return uint_to_float(pcg4d(rng_state).x);
}

vec2 rand2(inout uvec4 rng_state) {
    rng_state.w++;
    uvec4 pcg = pcg4d(rng_state);
    return vec2(uint_to_float(pcg.x), uint_to_float(pcg.y));
}


vec3 rand3(inout uvec4 rng_state) {
    rng_state.w++;
    uvec4 pcg = pcg4d(rng_state);
    return vec3(uint_to_float(pcg.x), uint_to_float(pcg.y), uint_to_float(pcg.z));
}

vec4 rand4(inout uvec4 rng_state) {
    rng_state.w++;
    uvec4 pcg = pcg4d(rng_state);
    return vec4(uint_to_float(pcg.x), uint_to_float(pcg.y), uint_to_float(pcg.z), uint_to_float(pcg.w));
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

#endif