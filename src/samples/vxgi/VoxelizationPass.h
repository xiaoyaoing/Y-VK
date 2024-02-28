#pragma once
#include "RenderGraph/RenderGraph.h"
#include "VoxelRegion.h"
#include "VxgiCommon.h"
#include "Core/BoundingBox.h"
#include "Scene/Scene.h"
#include "shaders/vxgi/vxgi_common.h"

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