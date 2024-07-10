#pragma once
#include "RenderPassBase.h"
#include "Core/PipelineLayout.h"

#include <memory>

class GBufferPass : public PassBase {
public:
    void render(RenderGraph& rg) override;
    void init() override;
    void updateGui() override;

private:
    std::unique_ptr<PipelineLayout> mPipelineLayout{nullptr};
    uint32_t frameIndex{0};
    uint32_t jitter{false};
    uint32_t stochastic{0};
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
    uint32_t frameIndex{0};
};