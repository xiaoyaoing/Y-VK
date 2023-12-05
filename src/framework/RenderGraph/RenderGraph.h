#pragma once

#include "PassNode.h"
#include "VirtualResource.h"
#include "Core/RenderTarget.h"
#include "RenderGraphId.h"
#include "RenderGraphPass.h"
#include "RenderGraphTexture.h"
#include "Core/RenderPass.h"
#include "BlackBoard.h"
/**render Graph **/
/*
 * 1.add pass 需要指明pass的输入 注册RenderGraphTexture 这一步会添加passNode 这里node会根据传入的RenderGraphPass::Descriptor创建RenderTarget
 * 2.指明pass node在execute时需要执行的东西 renderpass的创建根据node
 * 
 *
 *
 *
 */

// class RenderGraphPassBase;
// class RenderGraph;


class CommandBuffer;


class RenderGraph {
public:
    RenderGraph(Device &device);

    RenderGraph(RenderGraph &rhs) = delete;

    RenderGraph(RenderGraph &&rhs) = delete;


    RenderGraphHandle importTexture(const char *name, SgImage *hwTexture);

    class Builder {
    public:

        RenderGraphHandle readTexture(RenderGraphHandle input,
                                      RenderGraphTexture::Usage usage =
                                      RenderGraphTexture::DEFAULT_R_USAGE);

        RenderGraphHandle writeTexture(RenderGraphHandle output,
                                       RenderGraphTexture::Usage usage =
                                       RenderGraphTexture::DEFAULT_W_USAGE);

        // void addSubpass(const RenderGraphSubpassInfo&);

        void declare(const char *name,
                     const RenderGraphPassDescriptor &desc);


        Builder(PassNode *node, RenderGraph &renderGraph)
                : node(node),
                  renderGraph(renderGraph) {
        }

    protected:
        PassNode *node;
        RenderGraph &renderGraph;
    };


    RenderGraphHandle createTexture(const char *name, const RenderGraphTexture::Descriptor &desc = {});


    RenderGraphHandle addResource(VirtualResource *resource);

    RenderGraphHandle addTexture(RenderGraphTexture *texture);

    bool isWrite(RenderGraphHandle handle, const RenderPassNode *passNode) const;

    bool isRead(RenderGraphHandle handle, const RenderPassNode *passNode) const;

    void setUp();

    void execute(CommandBuffer &commandBuffer);

    Blackboard &getBlackBoard() const;

    
    template<typename Data, typename Setup, typename Execute>
    void addPass(const char *name, Setup setup, Execute &&execute) {
        auto pass = new RenderGraphPass<Data, Execute>(std::forward<Execute>(execute));
        Builder builder(addPassImpl(name, pass), *this);
        setup(builder, pass->getData());
    }

    void addPresentPass(RenderGraphHandle textureId);

    RenderGraphTexture *getResource(RenderGraphHandle handle) const;

    void compile();

    Device &getDevice() const;

    ~RenderGraph() {
        for (const auto &passNode: mPassNodes)
            delete passNode;
        for (const auto &resource: mVirtualResources)
            delete resource;
    }

private:
    std::vector<RenderGraphNode *> getInComingNodes(RenderGraphNode *node) const;

    std::vector<RenderGraphNode *> getOutComingNodes(RenderGraphNode *node) const;

    PassNode * addPassImpl(const char *name, RenderGraphPassBase *base) {
        auto node = new RenderPassNode(*this, name, base);
        mPassNodes.emplace_back(node);
        return node;
    }

    struct ResourceSlot {
        using Version = RenderGraphHandle::Version;
        using Index = int16_t;
        Index rid = 0; // VirtualResource* index in mResources
        Index nid = 0; // ResourceNode* index in mResourceNodes
        Index sid = -1; // ResourceNode* index in mResourceNodes for reading subresource's parent
        Version version = 0;
    };


    struct Edge {
        PassNode *pass{nullptr};
        RenderGraphTexture *texture{nullptr};
        RenderGraphTexture::Usage usage{};
        bool read{true};
    };

    std::vector<const Edge *> getEdges(RenderGraphNode *node) const;


    std::unique_ptr<Blackboard> mBlackBoard{};
    std::vector<PassNode *> mPassNodes{};
    std::vector<VirtualResource *> mVirtualResources{};
    std::vector<RenderGraphTexture *> mTextures{};
    std::vector<PassNode *>::iterator mActivePassNodesEnd;

    std::vector<ResourceSlot> mResourceSlots{};
    Device &device;


    std::vector<Edge> edges;

    // std::vector<std::unique_ptr<Vi>>
};
