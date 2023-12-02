#pragma once

#include <unordered_map>

#include "RenderGraphId.h"
#include "RenderGraphPass.h"
#include "RenderGraphTexture.h"
#include "Core/RenderTarget.h"


class RenderGraph;


struct RenderGraphSubpassInfo {
    std::vector<RenderGraphHandle> inputAttachments{};

    std::vector<RenderGraphHandle> outputAttachments{};
};

struct RenderGraphPassDescriptor {
    std::vector<RenderGraphHandle> textures;
    std::vector<RenderGraphSubpassInfo> subpasses;

    size_t getSubpassCount() const {
        return subpasses.size();
    }

    void addSubpass(const RenderGraphSubpassInfo &subpassInfo) {
        subpasses.push_back(subpassInfo);
    }
};


class PassNode : public RenderGraphNode {
public:
    virtual void execute(RenderGraph &renderGraph, CommandBuffer &commandBuffer) = 0;

    ~PassNode() override = default;


    void resolveTextureUsages(RenderGraph &renderGraph, CommandBuffer &commandBuffer);

    std::vector<RenderGraphTexture *> devirtualize; // resources need to be create before executing
    std::vector<RenderGraphTexture *> destroy; // resources need to be destroy after executing

    void addTextureUsage(const RenderGraphTexture *texture, RenderGraphTexture::Usage usage);

protected:
    std::unordered_map<const RenderGraphTexture *, TextureUsage> textureUsages;
};

class PresentPassNode : public PassNode {
    void execute(RenderGraph &renderGraph, CommandBuffer &commandBuffer) override;
};


class RenderPassNode final : public PassNode {
    virtual void declareRenderTarget(const char *name, const RenderGraphPassDescriptor &descriptor);

public:
    void execute(RenderGraph &renderGraph, CommandBuffer &commandBuffer) override;

    RenderPassNode(RenderGraph &renderGraph, const char *name, RenderGraphPassBase *base);

    ~RenderPassNode() override {
        delete mRenderPass;
    }

    void declareRenderPass(const char *name, const RenderGraphPassDescriptor &descriptor);

private:
    class RenderPassData {
    public:
        static constexpr size_t ATTACHMENT_COUNT = 6;
        const char *name = {};
        bool imported = false;

        RenderGraphPassDescriptor desc;

        void devirtualize(RenderGraph &renderGraph, const RenderPassNode &node);

        std::unique_ptr<RenderTarget> renderTarget;

        RenderTarget &getRenderTarget();
    };

    RenderPassData renderPassData{};
    RenderGraphPassBase *mRenderPass{nullptr};
    const char *name;
};
