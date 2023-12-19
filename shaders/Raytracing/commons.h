#ifndef COMMONS_HOST_DEVICE
#define COMMONS_HOST_DEVICE

#ifdef __cplusplus
#include <glm/glm.hpp>
using vec2 = glm::vec2;
using ivec3 = glm::ivec3;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat4 = glm::mat4; 
using uvec4 = glm::uvec4;
using ivec2 = glm::ivec2;
using uint = unsigned int;
using uvec2 = glm::uvec2;
#else
#define NAMESPACE_BEGIN(name)
#define NAMESPACE_END()
#define ALIGN16
#endif


//For code used in shader,use "_" code style 

struct  SceneDesc{
    uint64_t vertex_addr;
    uint64_t index_addr;
    uint64_t normal_addr;
    uint64_t uv_addr;
    uint64_t material_addr;
    uint64_t prim_info_addr;

    // NEE
    uint64_t mesh_lights_addr;
   // uint64_t light_vis_addr;
};

struct SceneUbo {
    mat4 viewInverse;
    mat4 projInverse;
    vec4 lightPos;
};


struct RTMaterial {
    vec3 emissiveFactor;
    vec3 albedo;

    uint texture_id;
    uint bsdf_type;
};


struct RTLight
{
    mat4 world_matrix;
    vec3 L;
    uint prim_idx;
    // vec3 to_use;
    // uint light_flags;
    
};

struct RTPrimitive
{
    uint material_index;
    uint vertex_offset;
    uint vertex_count;
    uint index_offset;
    uint index_count;
    mat4 world_matrix;
};

// struct PrimitiveMeshInfo
// {
//     uint triangle_count;
//     float total_area;
// };
#endif
