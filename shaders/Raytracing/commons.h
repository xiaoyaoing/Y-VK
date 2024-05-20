#ifndef COMMONS_HOST_DEVICE
#define COMMONS_HOST_DEVICE

#ifdef __cplusplus
#include <glm/glm.hpp>
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

#else
#define NAMESPACE_BEGIN(name)
#define NAMESPACE_END()
#define ALIGN16
#endif

#define LIGHT_TYPE_AREA 1
#define LIGHT_TYPE_INFINITE 2

//For code used in shader,use "_" code style

struct SceneDesc {
    uint envmap_idx;
    uint padding;
    
    uint64_t vertex_addr;
    uint64_t index_addr;
    uint64_t normal_addr;
    uint64_t uv_addr;
    uint64_t material_addr;
    uint64_t prim_info_addr;

    // NEE
    uint64_t mesh_lights_addr;

    uint64_t env_sampling_addr;

    
    uint64_t restir_temporal_reservoir_addr;
    uint64_t restir_spatial_reservoir_addr;
    uint64_t restir_pass_reservoir_addr;
    uint64_t restir_color_storage_addr;
    
    uint64_t gbuffer_addr;
};

struct SceneUbo {
    mat4 view;
    mat4 proj;
    mat4 viewInverse;
    mat4 projInverse;
    vec4 lightPos;
    mat4 prev_view;
    mat4 prev_proj;
};

struct RTMaterial {
    vec3  emissiveFactor;
    vec3  albedo;
    int   texture_id;
    uint  bsdf_type;

    vec3  eta;
    float roughness;
    
    vec3  k;
    float _diffuseFresnel;

    vec3  _scaledSigmaA;
    float _avgTransmittance;

    float ior;
    vec3 padding;
};

struct RTLight {
    mat4 world_matrix;
    vec3 L;
    uint prim_idx;
    vec3 position;
    uint light_type;
    vec3 padding;
    uint light_texture_id;
};

struct RTPrimitive {
    uint material_index;//4
    uint vertex_offset; //8
    uint vertex_count;  //12
    uint index_offset;

    uint  index_count;
    float area;//24
    uint  light_index;
    uint  padding2;

    mat4     world_matrix;//104
    uint64_t area_distribution_buffer_addr;
    uint     padding3;
    uint     padding4;
    //  uint64_t padding_64;
};

struct EnvAccel
{
    uint  alias;
    float q;
    float pdf;
    float aliasPdf;
};

#define RT_BSDF_TYPE_DIFFUSE   0
#define RT_BSDF_TYPE_MIRROR    1
#define RT_BSDF_TYPE_DIELCTRIC 2
#define RT_BSDF_TYPE_CONDUCTOR 3
#define RT_BSDF_TYPE_PLASTIC   4
#define RT_BSDF_TYPE_PRINCIPLE 5

#define RT_LIGHT_TYPE_AREA     0
#define RT_LIGHT_TYPE_INFINITE 1
#define RT_LIGHT_TYPE_POINT    2

#define RT_BSDF_LOBE_DIFFUSE    1u
#define RT_BSDF_LOBE_SPECULAR   1u << 1
#define RT_BSDF_LOBE_GLOSSY     1u << 2
#define RT_BSDF_LOBE_REFLECTION 1u << 3
#define RT_BSDF_LOBE_REFRACTION 1u << 4

// struct PrimitiveMeshInfo
// {
//     uint triangle_count;
//     float total_area;
// };
#endif