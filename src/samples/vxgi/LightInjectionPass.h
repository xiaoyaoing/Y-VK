#pragma once
#include "ClipmapUpdatePolicy.h"
#include "VxgiCommon.h"
#include "RenderPasses/RenderPassBase.h"

class LightInjectionPass : public PassBase {
public:
    void render(RenderGraph& rg) override;
    void init() override;
    void updateGui() override;

protected:
    std::unique_ptr<PipelineLayout> mLightInjectionPipelineLayout{nullptr};
    // std::unique_ptr<SgImage>        mVoxelizationImage{nullptr};
    std::unique_ptr<SgImage>             mLightInjectionImage{nullptr};
    ClipmapUpdatePolicy*                 mClipmapUpdatePolicy{nullptr};
    uint32_t                             frameIndex{0};
    std::vector<std::unique_ptr<Buffer>> mVoxelParamBuffers{};
    bool                                 injectLight[6] = {true, false, false, false, true, false};
};