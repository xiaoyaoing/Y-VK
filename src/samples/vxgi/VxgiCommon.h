#pragma once

#include "Core/BoundingBox.h"
#include "RenderGraph/RenderGraph.h"
#include <vector>

class Scene;

// struct VxgiContext {
//     Scene*              scene;
//     std::vector<BBox>*  bboxes;
//     Device&             device;
//     static VxgiContext& getInstance();
// };

#define VOXEL_RESOLUTION 128
#define CLIP_MAP_LEVEL   6

class VxgiPassBase {
public:
    virtual void render(RenderGraph& rg) = 0;
    virtual void init()                  = 0;
    virtual void update() {}
    virtual ~VxgiPassBase() = default;
};

class VxgiPtrManangr {
public:
    template<typename T>
    T* fetchPtr(const std::string_view name) {
        return static_cast<T*>(mPointersMap[name]);
    }

    template<typename T>
    void putPtr(const std::string_view name, T* ptr) {
        mPointersMap[name] = ptr;
    }

protected:
    std::unordered_map<std::string_view, void*> mPointersMap;
};

struct VxgiConfig {
    inline static int voxelResolution{128};
    inline static int clipMapLevelCount{6};
    inline static int level0MaxExtent{16};
};

inline VxgiPtrManangr* g_manager = nullptr;
// VxgiPtrManangr * VxgiPtrManangr::gManager = nullptr;