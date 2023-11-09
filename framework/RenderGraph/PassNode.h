#pragma once

#include <unordered_map>

#include "RenderGraphId.h"
#include "RenderGraphPass.h"
#include "RenderGraphTexture.h"
#include "RenderTarget.h"


class RenderGraph;

struct RenderGraphPassDescriptor
{
    std::vector<RenderGraphHandle> color;
    std::vector<LoadStoreInfo> loadStoreInfos;
    RenderGraphHandle depth;
    RenderGraphHandle stencil;


    // std::vector<RenderGraphSubPassInfo> subpasses;
    // Attachments attachments{};
    // Viewport viewport{};
    // math::float4 clearColor{};
    // uint8_t samples = 0; // # of samples (0 = unset, default)
    // backend::TargetBufferFlags clearFlags{};
    // backend::TargetBufferFlags discardStart{};
};


class PassNode : public RenderGraphNode
{
public:
    virtual void execute(RenderGraph& renderGraph, CommandBuffer& commandBuffer) = 0;

    ~PassNode() override = default;

    void addTextureUsage(RenderGraphHandle id, RenderGraphTexture::Usage usage);

    void resolveTextureUsages(RenderGraph& renderGraph, CommandBuffer& commandBuffer);

    std::vector<RenderGraphTexture*> devirtualize; // resources need to be create before executing
    std::vector<RenderGraphTexture*> destroy; // resources need to be destroy after executing

protected:
    struct hash
    {
        size_t operator()(const RenderGraphHandle& texture) const
        {
            return texture.getHash();
        }
    };

    // friend class RenderGraph::Builder;
    std::unordered_map<RenderGraphHandle, TextureUsage, hash> textureUsages;
};

class PresentPassNode : public PassNode
{
    void execute(RenderGraph& renderGraph, CommandBuffer& commandBuffer) override;
};


class RenderPassNode final : public PassNode
{
    virtual void declareRenderTarget(const char* name, const RenderGraphPassDescriptor& descriptor);

public:
    void execute(RenderGraph& renderGraph, CommandBuffer& commandBuffer) override;

    RenderPassNode(RenderGraph& renderGraph, const char* name, RenderGraphPassBase* base);

    ~RenderPassNode() override
    {
        delete mRenderPass;
    }

    void declareRenderPass(const char* name, const RenderGraphPassDescriptor& descriptor);

private:
    class RenderPassData
    {
    public:
        static constexpr size_t ATTACHMENT_COUNT = 6;
        const char* name = {};
        bool imported = false;

        RenderGraphPassDescriptor desc;

        //RenderGraphHandle attachmentInfo[ATTACHMENT_COUNT];


        void devirtualize(RenderGraph& renderGraph,const RenderPassNode & node);

        std::unique_ptr<RenderTarget> renderTarget;

        RenderTarget& getRenderTarget();
    };

    RenderPassData renderTargetData{};
    RenderGraphPassBase* mRenderPass{nullptr};
    const char* name;
};
