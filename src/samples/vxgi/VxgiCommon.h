#pragma once

#include "RenderGraph/RenderGraph.h"
#include <glm/glm.hpp>

class Scene;

// struct VxgiContext {
//     Scene*              scene;
//     std::vector<BBox>*  bboxes;
//     Device&             device;
//     static VxgiContext& getInstance();
// };

#define VOXEL_RESOLUTION     128
#define CLIP_MAP_LEVEL_COUNT 6

struct VxgiConfig {
    inline static int voxelResolution{128};
    inline static int clipMapLevelCount{6};
    inline static int level0MaxExtent{16};
};

struct alignas(16) VoxelizationParamater {
    //using vec3 and float/int layout avoid alignment problem of vec3 in std140 glsl layout
    glm::vec3 prevClipmapMinWorldPos{};
    int       clipmapLevel;
    glm::vec3 prevClipmapMaxWorldPos{};
    float     voxelSize;
    glm::vec3 clipmapMinWorldPos;
    int       voxelResolution;
    glm::vec3 clipmapMaxWorldPos;
    float     maxExtentWorld;
};

// VxgiPtrManangr * VxgiPtrManangr::gManager = nullptr;