#pragma once
#include "VxgiCommon.h"
#include "Core/Images/Sampler.h"
#include "RenderPasses/RenderPassBase.h"
class CopyAlphaPass : public PassBase {
public:
    void render(RenderGraph& rg) override;
    void init() override;

protected:
    std::unique_ptr<PipelineLayout> mPipelineLayout;
    std::unique_ptr<Sampler>        mSampler;
};