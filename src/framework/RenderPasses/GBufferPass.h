#pragma once
#include "RenderPassBase.h"
#include "Core/PipelineLayout.h"

#include <memory>

class GBufferPass : public PassBase {
public:
    void render(RenderGraph& rg) override;
    void init() override;

private:
    std::unique_ptr<PipelineLayout> mPipelineLayout{nullptr};
    // Device&                         device;
};

class LightingPass : public PassBase {
public:
    void render(RenderGraph& rg) override;
    void init() override;

private:
    std::unique_ptr<PipelineLayout> mPipelineLayout{nullptr};
};

class IBLLightingPass : public PassBase {
public:
    void render(RenderGraph& rg) override;
    void updateGui() override;

protected:
    int debugMode = 0;
};