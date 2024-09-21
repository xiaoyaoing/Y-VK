#pragma once

#include "Core/BoundingBox.h"
#include "ClipmapRegion.h"
#include "ClipmapUpdatePolicy.h"

#include <memory>

class Scene;

// struct VxgiContext {
//     Scene*              scene;
//     std::vector<BBox>*  bboxes;
//     Device&             device;
//     static VxgiContext& getInstance();
// };

struct VxgiContext {
    //static VxgiContext& getInstance();
protected:
    struct VxgiContextImpl;
    VxgiContext();
    VxgiContextImpl* mImpl = nullptr;
    static VxgiContext & getInstance();
public:
    static std::vector<ClipmapRegion>& getClipmapRegions();
    static void setClipmapRegions(std::vector<ClipmapRegion>& regions);
    static ClipmapUpdatePolicy& getClipmapUpdatePolicy();
    static std::vector<BBox>& getBBoxes();
};

#define VOXEL_RESOLUTION     128
#define CLIP_MAP_LEVEL_COUNT 6

struct VxgiConfig {
    inline static int voxelResolution{128};
    inline static int clipMapLevelCount{6};
    inline static int level0MaxExtent{16};
};


// VxgiPtrManangr * VxgiPtrManangr::gManager = nullptr;