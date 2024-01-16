#pragma once

#include <unordered_map>

#include "RenderGraphId.h"
#include "RenderGraphPass.h"
#include "RenderGraphTexture.h"
#include "Core/RenderTarget.h"
#include "Core/RayTracing/Accel.h"

class PipelineLayout;
class RenderGraph;

struct RenderGraphSubpassInfo {
    std::vector<RenderGraphHandle> inputAttachments{};

    std::vector<RenderGraphHandle> outputAttachments{};

    bool disableDepthTest{false};
};

struct RenderGraphPassDescriptor {
    std::vector<RenderGraphHandle>      textures;
    std::vector<RenderGraphSubpassInfo> subpasses;

    size_t getSubpassCount() const {
        return subpasses.size();
    }

    RenderGraphPassDescriptor& addSubpass(const RenderGraphSubpassInfo& subpassInfo) {
        subpasses.push_back(subpassInfo);
        return *this;
    }
};

class PassNode : public RenderGraphNode {
public:
    PassNode(const char* passName);
    ~PassNode() override = default;
    void resolveTextureUsages(RenderGraph& renderGraph, CommandBuffer& commandBuffer);
    void addResourceUsage(ResourceNode* texture, uint16_t usage);

    virtual void execute(RenderGraph& renderGraph, CommandBuffer& commandBuffer) = 0;
    bool         active() { return mActive & getRefCount() > 0; }
    void         setActive(bool active) { mActive = active; }

    std::vector<ResourceNode*> devirtualize{};// resources need to be create before executing
    std::vector<ResourceNode*> destroy{};     // resources need to be destroy after executing
    const char*                mPassName{nullptr};

protected:
    bool                                        mActive{true};
    std::unordered_map<ResourceNode*, uint16_t> mResourceUsage;
};

class ImageCopyPassNode : public PassNode {
public:
    void execute(RenderGraph& renderGraph, CommandBuffer& commandBuffer) override;
    ImageCopyPassNode(RenderGraphHandle src, RenderGraphHandle dst);

protected:
    RenderGraphHandle src, dst;
};

class RenderPassNode final : public PassNode {
    virtual void declareRenderTarget(const char* name, const RenderGraphPassDescriptor& descriptor);

public:
    void execute(RenderGraph& renderGraph, CommandBuffer& commandBuffer) override;

    RenderPassNode(RenderGraph& renderGraph, const char* name, RenderGraphPassBase* base);

    ~RenderPassNode() override {
        delete mRenderPass;
    }

    void declareRenderPass(const RenderGraphPassDescriptor& descriptor);

private:
    class RenderPassData {
    public:
        static constexpr size_t ATTACHMENT_COUNT = 6;
        const char*             name             = {};
        bool                    imported         = false;

        RenderGraphPassDescriptor desc;

        void devirtualize(RenderGraph& renderGraph, const RenderPassNode& node);

        std::unique_ptr<RenderTarget> renderTarget;

        RenderTarget& getRenderTarget();
    };

    RenderPassData       renderPassData{};
    RenderGraphPassBase* mRenderPass{nullptr};
    const char*          name;
};

class ComputePassNode final : public PassNode {
public:
    void execute(RenderGraph& renderGraph, CommandBuffer& commandBuffer) override;
    ComputePassNode(RenderGraph& renderGraph, const char* name, ComputeRenderGraphPass* base);
    ~ComputePassNode() override { delete mPass; }

private:
    ComputeRenderGraphPass* mPass{nullptr};
};

class RayTracingPassNode final : public PassNode {
public:
    void execute(RenderGraph& renderGraph, CommandBuffer& commandBuffer) override;
    RayTracingPassNode(RenderGraph& renderGraph, const char* name, RaytracingRenderGraphPass* base);
    ~RayTracingPassNode() override { delete mPass; }
    // ~RayTracingPassNode() override;
private:
    RaytracingRenderGraphPass* mPass{nullptr};
};