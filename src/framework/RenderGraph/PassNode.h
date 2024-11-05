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
    VkExtent2D extent2D = {0, 0};

    size_t getSubpassCount() const {
        return subpasses.size();
    }

    RenderGraphPassDescriptor& addSubpass(const RenderGraphSubpassInfo& subpassInfo) {
        subpasses.push_back(subpassInfo);
        return *this;
    }
    RenderGraphPassDescriptor& setTextures(const std::vector<RenderGraphHandle>& textures) {
        this->textures = textures;
        return *this;
    }
    RenderGraphPassDescriptor(const std::vector<RenderGraphHandle>& textures, const RenderGraphSubpassInfo& subpass) : textures(textures), subpasses({subpass}) {
    }
    RenderGraphPassDescriptor() = default;
};

class PassNode : public RenderGraphNode {
public:
    PassNode(const std::string& passName);
    ~PassNode() override = default;
    void resolveResourceUsages(RenderGraph& renderGraph, CommandBuffer& commandBuffer);
    void addResourceUsage(RenderGraphHandle handle, uint16_t usage);
    bool active() { return mActive & getRefCount() > 0; }
    void setActive(bool active) { mActive = active; }

    virtual void                   execute(RenderGraph& renderGraph, CommandBuffer& commandBuffer) = 0;
    virtual RenderPassType getType() const { return RenderPassType::UNDEFINED; }

    std::vector<ResourceNode*> devirtualize{};// resources need to be create before executing
    std::vector<ResourceNode*> destroy{};     // resources need to be destroy after executing

protected:
    bool                                        mActive{true};
    std::unordered_map<RenderGraphHandle, uint16_t,RenderGraphHandle::Hash> mResourceUsage;
};

class ImageCopyPassNode final : public PassNode {
public:
    void execute(RenderGraph& renderGraph, CommandBuffer& commandBuffer) override;
    ImageCopyPassNode(RenderGraphHandle src, RenderGraphHandle dst);
    RenderPassType getType() const override;

protected:
    RenderGraphHandle src, dst;
};

class GraphicsPassNode final : public PassNode {
    virtual void declareRenderTarget(const std::string& name, const RenderGraphPassDescriptor& descriptor);

public:
    void execute(RenderGraph& renderGraph, CommandBuffer& commandBuffer) override;

    GraphicsPassNode(RenderGraph& renderGraph, const std::string& name, RenderGraphPassBase* base);

    ~GraphicsPassNode() override {
        delete mRenderPass;
    }

    void                   declareRenderPass(const RenderGraphPassDescriptor& descriptor);
    RenderPassType getType() const override;

private:
    class RenderPassData {
    public:
        static constexpr size_t                                                ATTACHMENT_COUNT = 6;
        std::string                                                            name             = {};
        bool                                                                   imported         = false;
        std::unordered_map<RenderGraphHandle, size_t, RenderGraphHandle::Hash> attachment_textures;

        RenderGraphPassDescriptor desc;

        void devirtualize(RenderGraph& renderGraph, const GraphicsPassNode& node);

        std::unique_ptr<RenderTarget> renderTarget;

        RenderTarget&                                                                 getRenderTarget();
        const std::unordered_map<RenderGraphHandle, size_t, RenderGraphHandle::Hash>& getAttachmentTextures() const {
            return attachment_textures;
        }
    };

    RenderPassData       renderPassData{};
    RenderGraphPassBase* mRenderPass{nullptr};
    std::string          name;
};

class ComputePassNode final : public PassNode {
public:
    void execute(RenderGraph& renderGraph, CommandBuffer& commandBuffer) override;
    ComputePassNode(RenderGraph& renderGraph, const std::string& name, ComputeRenderGraphPass* base);
    ~ComputePassNode() override { delete mPass; }
    RenderPassType getType() const override;

private:
    ComputeRenderGraphPass* mPass{nullptr};
};

class RayTracingPassNode final : public PassNode {
public:
    void execute(RenderGraph& renderGraph, CommandBuffer& commandBuffer) override;
    RayTracingPassNode(RenderGraph& renderGraph, const std::string& name, RaytracingRenderGraphPass* base);
    ~RayTracingPassNode() override { delete mPass; }
    RenderPassType getType() const override;
    // ~RayTracingPassNode() override;
private:
    RaytracingRenderGraphPass* mPass{nullptr};
};