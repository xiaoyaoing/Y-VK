#pragma once
#include "RenderGraphId.h"
#include "RenderGraphPass.h"
#include "RenderGraphTexture.h"
#include "RenderTarget.h"


class RenderGraph;

struct RenderGraphPassDescriptor
{
    std::vector<RenderGraphId<RenderGraphTexture>> color;
    RenderGraphId<RenderGraphTexture> depth;
    RenderGraphId<RenderGraphTexture> stencil;
    // Attachments attachments{};
    // Viewport viewport{};
    // math::float4 clearColor{};
    // uint8_t samples = 0; // # of samples (0 = unset, default)
    // backend::TargetBufferFlags clearFlags{};
    // backend::TargetBufferFlags discardStart{};
};


class PassNode
{
public:
    PassNode(RenderGraph& renderGraph, const char* name, RenderGraphPassBase* base);
    virtual void execute(RenderGraph& renderGraph, CommandBuffer& commandBuffer);

protected:
};


class RenderPassNode : public PassNode
{
    virtual void declareRenderTarget(const char* name, const RenderGraphPassDescriptor& descriptor);

    class RenderPassData
    {
    public:
        static constexpr size_t ATTACHMENT_COUNT = 6;
        const char* name = {};
        bool imported = false;

        RenderGraphPassDescriptor desc;

        RenderGraphId<RenderGraphTexture> attachmentInfo[ATTACHMENT_COUNT] = {};


        void devirtualize(RenderGraph& renderGraph);

        RenderTarget renderTarget;
        RenderTarget& getRenderTarget();
    };

public:
    void execute(RenderGraph& renderGraph, CommandBuffer& commandBuffer) override;

private:
    RenderGraphPassBase* mRenderPass;
    RenderPassData renderTargetData;
};

class PresentPassNode : public PassNode
{
};
