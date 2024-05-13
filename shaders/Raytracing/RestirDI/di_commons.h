#ifndef DI_COMMONS_H
#define DI_COMMONS_H
#ifdef __cplusplus
using vec2  = glm::vec2;
using ivec3 = glm::ivec3;
using vec3  = glm::vec3;
using vec4  = glm::vec4;
using mat4  = glm::mat4;
using uvec4 = glm::uvec4;
using ivec2 = glm::ivec2;
using uint  = unsigned int;
using uvec2 = glm::uvec2;
#define ALIGN16 alignas(16)

#endif

struct RestirData {
    uint light_idx;
    
};

struct RestirDIPC {
    vec3 sky_col;
    uint frame_num;
    uint light_num;
    uint max_depth;
    uint min_depth;
    uint enable_sample_bsdf;
    uint enable_sample_light;
    uint do_temporal_reuse;
    uint do_spatial_reuse;
};

struct RestirReservoir {
    float w_sum;
    float W;
    uint m;
    RestirData s;
    float p_hat;
    float pdf;
};
#endif