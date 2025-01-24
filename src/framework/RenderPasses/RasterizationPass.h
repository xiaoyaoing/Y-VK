#pragma once
#include "RenderPassBase.h"
#include "Core/PipelineLayout.h"
#include "RenderGraph/RenderGraphId.h"

#include <memory>

class GBufferPass : public PassBase {
public:
    void render(RenderGraph& rg) override;
    void renderToBuffer(RenderGraph& rg,RenderGraphHandle outputBuffer,RenderGraphHandle directLightingImage = RenderGraphHandle::InvalidHandle());

private:

    // Device&                         device;
};

class VBufferPass : public PassBase {
public:
    void render(RenderGraph& rg) override;
};


class LightingPass : public PassBase {
public:
    void render(RenderGraph& rg) override;
};

class ForwardPass : public PassBase {
public:
    void render(RenderGraph& rg) override;
};

class IBLLightingPass : public PassBase {
public:
    void render(RenderGraph& rg) override;
    void updateGui() override;

protected:
    int debugMode = 0;
};
