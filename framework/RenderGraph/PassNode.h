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
    virtual void execute(RenderGraph& renderGraph, CommandBuffer& commandBuffer) = 0;
    virtual ~PassNode() = default;

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

        //RenderGraphId<RenderGraphTexture> attachmentInfo[ATTACHMENT_COUNT];


        void devirtualize(RenderGraph& renderGraph);

        std::unique_ptr<RenderTarget> renderTarget;

        RenderTarget& getRenderTarget();
    };

public:
    void execute(RenderGraph& renderGraph, CommandBuffer& commandBuffer) override;

    RenderPassNode(RenderGraph& renderGraph, const char* name, RenderGraphPassBase* base);

    ~RenderPassNode() override
    {
        delete mRenderPass;
    }

private:
    friend class RenderGraph;

    RenderGraphPassBase* mRenderPass{nullptr};
    RenderPassData renderTargetData;
};

class PresentPassNode : public PassNode
{
};
