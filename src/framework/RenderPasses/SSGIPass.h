#pragma once

#include "RenderPassBase.h"
class SSGIPass : public PassBase {
public:
    void render(RenderGraph& rg) override;
    void init() override;
    void updateGui() override;
    void update() override;
    ~SSGIPass() override = default;
};
