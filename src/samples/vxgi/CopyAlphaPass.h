#pragma once
#include "VxgiCommon.h"
#include "Core/Images/Sampler.h"
class CopyAlphaPass : public VxgiPassBase {
public:
    void render(RenderGraph& rg) override;
    void init() override;

protected:
    std::unique_ptr<PipelineLayout> mPipelineLayout;
    std::unique_ptr<Sampler> mSampler;
};