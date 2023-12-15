#pragma once

#define ALIGN16 alignas(16)

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
#endif


//For code used in shader,use "_" code style 

struct ALIGN16  SceneDesc{
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
    uint32_t light_flags;
    uint32_t prim_idx;
    mat4 world_matrix;
    vec3 L;
};

struct RTPrimitive
{
    uint32_t material_index;
    uint32_t vertex_offset;
    uint32_t vertex_count;
    uint32_t index_offset;
    uint32_t index_count;
    glm::mat4 world_matrix;
};

// struct PrimitiveMeshInfo
// {
//     uint32_t triangle_count;
//     float total_area;
// };
