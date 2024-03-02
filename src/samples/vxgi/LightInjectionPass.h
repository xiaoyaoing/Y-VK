#pragma once
#include "VxgiCommon.h"

class LightInjectionPass : public VxgiPassBase {
public:
    void render(RenderGraph& rg) override;
    void init() override;

protected:
    std::unique_ptr<PipelineLayout> mLightInjectionPipelineLayout{nullptr};
    // std::unique_ptr<SgImage>        mVoxelizationImage{nullptr};
    std::unique_ptr<SgImage> mLightInjectionImage{nullptr};
    uint32_t                 frameIndex{0};
};