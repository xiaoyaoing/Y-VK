#pragma once
#include "RenderPassBase.h"
#include "Core/PipelineLayout.h"

#include <memory>
class Sampler;
class ShadowMapPass : public PassBase {
public:
    ShadowMapPass();
    void render(RenderGraph& rg) override;
    void init() override;
protected:
    std::unique_ptr<PipelineLayout> mPipelineLayout;
    Sampler & mSampler;
};
