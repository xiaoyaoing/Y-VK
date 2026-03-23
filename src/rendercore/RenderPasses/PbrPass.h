#pragma once
#include "RenderPasses/RenderPassBase.h"

class PbrPass : public PassBase {
public:
    void render(RenderGraph& rg) override;
    void init() override;
    void updateGui() override;
};
