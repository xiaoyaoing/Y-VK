#pragma once
#include "VxgiCommon.h"

class FinalLightingPass : public VxgiPassBase {
public:
    void init() override;
    void render(RenderGraph& rg) override;
};
inline void FinalLightingPass::init() {
}
inline void FinalLightingPass::render(RenderGraph& rg) {
}