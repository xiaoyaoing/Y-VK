#pragma once
#include "VxgiCommon.h"

class FinalLightingPass : public VxgiPassBase {
public:
    void init() override;
    void render(RenderGraph& rg) override;
protected:
    std::unique_ptr<PipelineLayout> mFinalLightingPipelineLayout{nullptr};
};
