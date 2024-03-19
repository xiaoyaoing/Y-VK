#pragma once
#include "RenderGraph/RenderGraph.h"
#include "ClipmapRegion.h"
#include "VxgiCommon.h"
#include "Core/BoundingBox.h"
#include "Scene/Scene.h"
#include "shaders/vxgi/vxgi_common.h"

class VoxelizationPass : public VxgiPassBase {

public:
    void       initClipRegions();
    void       init() override;
    void       render(RenderGraph& rg) override;
    glm::ivec3 computeChangeDeltaV(uint32_t clipmapLevel);
    void       fillRevoxelizationRegions(uint32_t clipLevel, const BBox& boundingBox);
    void       updateVoxelization();

private:
    std::vector<BBox>*                      mBBoxes{};
    std::vector<ClipmapRegion>              mClipRegions{};
    std::vector<std::vector<ClipmapRegion>> mRevoxelizationRegions{};

    uint32_t                             mCurBufferIndex{0};
    std::vector<std::unique_ptr<Buffer>> mVoxelParamBuffers{};

    // std::vector<std::unique_ptr<Buffer>> mVoxelParamBuffer{nullptr};
    VoxelizationParamater mVoxelParam{};

    std::unique_ptr<SgImage>        mVoxelizationImage{nullptr};
    std::unique_ptr<SgImage>        mVoxelRadianceImage{nullptr};
    std::unique_ptr<PipelineLayout> mVoxelizationPipelineLayout{nullptr};

    bool  mFullRevoxelization{false};
    bool  mInitVoxelization{false};
    ivec3 mMinVoxelChange{2, 2, 2};

    // std::unique_ptr<ClipMapCleaner> mClipMapCleaner{nullptr};
};
