#pragma once
#include "RenderGraph/RenderGraph.h"
#include "VoxelRegion.h"
#include "VxgiCommon.h"
#include "Core/BoundingBox.h"
#include "Scene/Scene.h"
#include "shaders/vxgi/vxgi_common.h"

struct VxgiConfig {
    int voxelResolution{128};
    int clipMapLevel{6};
    int level0MaxExtent{16};
};

#define VOXEL_RESOLUTION 128
#define CLIP_MAP_LEVEL   6

class VoxelizationPass : public VxgiPassBase {

public:
    void       init() override;
    void       render(RenderGraph& rg) override;
    glm::ivec3 computeChangeDeltaV(uint32_t clipmapLevel);

private:
    std::vector<BBox>*       mBBoxes{};
    std::vector<VoxelRegion> mClipRegions{};

    std::unique_ptr<Buffer> mVoxelParamBuffer{nullptr};
    VoxelizationParamater   mVoxelParam{};

    std::unique_ptr<SgImage>        mVoxelizationImage{nullptr};
    std::unique_ptr<SgImage>        mVoxelRadianceImage{nullptr};
    std::unique_ptr<PipelineLayout> mVoxelizationPipelineLayout{nullptr};
};