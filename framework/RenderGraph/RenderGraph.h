#pragma once

#include "PassNode.h"
#include "VirtualResource.h"
#include "RenderTarget.h"
#include "RenderGraphId.h"
#include "RenderGraphPass.h"
#include "RenderGraphTexture.h"
#include "RenderPass.h"
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


struct RenderPassDesc
{
    std::vector<Attachment> colorAttachments;
    Attachment depthAttachment;
};


// struct Attachments {
//     union {
//         RenderGraphHandle array[ATTACHMENT_COUNT] = {};
//         struct {
//             RenderGraphHandle color[backend::MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT];
//             RenderGraphHandle depth;
//             RenderGraphHandle stencil;
//         };
//     };
// struct RenderPassData
// {
//     VkViewport viewport{};
//     VkClearValue clearColor{};
//     uint8_t samples = 0; // # of samples (0 = unset, default)
//     //  backend::TargetBufferFlags clearFlags{};
//     //  backend::TargetBufferFlags discardStart{};
// };

struct RenderGraphSubpassInfo
{
    std::vector<RenderGraphHandle> inputAttachments{};

    std::vector<RenderGraphHandle> outputAttachments{};
};


class RenderGraph
{
public:
    RenderGraph(Device& device);


    // template <typename RESOURCE>
    // RenderGraphId<RESOURCE> create(char* name, const typename RESOURCE::Descriptor& desc)
    // {
    //     VirtualResource* virtualResource = Resource<RESOURCE>(name, desc);
    //     return RenderGraphId<RESOURCE>(addResource(virtualResource));
    // }

    RenderGraphHandle importTexture(const char* name, sg::SgImage* hwTexture);

    class Builder
    {
    public:
        // RenderGraphHandle read(RenderGraphHandle resource, PassNode* node,);
        //
        // RenderGraphHandle write(RenderGraphHandle resource, PassNode* node);

        RenderGraphHandle readTexture(RenderGraphHandle input,
                                      RenderGraphTexture::Usage usage =
                                          RenderGraphTexture::DEFAULT_R_USAGE);
        RenderGraphHandle writeTexture(RenderGraphHandle output,
                                       RenderGraphTexture::Usage usage =
                                           RenderGraphTexture::DEFAULT_W_USAGE);


        void declare(const char* name,
                     const RenderGraphPassDescriptor& desc);

        //  void addSubpass();

        Builder(PassNode* node, RenderGraph& renderGraph)
            : node(node),
              renderGraph(renderGraph)
        {
        }

    protected:
        PassNode* node;
        RenderGraph& renderGraph;
    };

    // template <typename RESOURCE>
    // RenderGraphId<RESOURCE> create(const char* name, const typename RESOURCE::Descriptor& desc = {})
    // {
    //     VirtualResource* virtualResource = new Resource<RESOURCE>(name, desc);
    //     return RenderGraphId<RESOURCE>(addResource(virtualResource));
    // }

    RenderGraphHandle createTexture(const char* name, const RenderGraphTexture::Descriptor& desc = {});


    RenderGraphHandle addResource(VirtualResource* resource);

    RenderGraphHandle addTexture(RenderGraphTexture* texture);

    bool isWrite(RenderGraphHandle handle, const RenderPassNode* passNode) const;

    void setUp();

    void execute(CommandBuffer& commandBuffer);

    Blackboard& getBlackBoard();


    PassNode* addPassInternal(const char* name, RenderGraphPassBase* base)
    {
        //auto node = ;
        auto node = new RenderPassNode(*this, name, base);
        mPassNodes.emplace_back(node);
        return node;
        //renderGraphNodes.push_back(node);
    }


    template <typename Data, typename Setup, typename Execute>
    void addPass(const char* name, Setup setup, Execute&& execute)
    {
        auto pass = new RenderGraphPass<Data, Execute>(std::forward<Execute>(execute));
        Builder builder(addPassInternal(name, pass), *this);
        setup(builder, pass->getData());
    }

    void addPresentPass(RenderGraphHandle textureId);

    RenderGraphTexture* getResource(RenderGraphHandle handle) const;

    void compile();

    Device& getDevice() const;

    ~RenderGraph()
    {
        for (const auto& passNode : mPassNodes)
            delete passNode;
        for (const auto& resource : mVirtualResources)
            delete resource;
    }

private:
    std::vector<RenderGraphNode*> getInComingNodes(RenderGraphNode* node) const;
    std::vector<RenderGraphNode*> getOutComingNodes(RenderGraphNode* node) const;

    struct ResourceSlot
    {
        using Version = RenderGraphHandle::Version;
        using Index = int16_t;
        Index rid = 0; // VirtualResource* index in mResources
        Index nid = 0; // ResourceNode* index in mResourceNodes
        Index sid = -1; // ResourceNode* index in mResourceNodes for reading subresource's parent
        Version version = 0;
    };


    struct Edge
    {
        PassNode* pass{nullptr};
        RenderGraphTexture* texture{nullptr};
        RenderGraphTexture::Usage usage{};
        bool read{true};
    };

    std::unique_ptr<Blackboard> mBlackBoard{};
    std::vector<PassNode*> mPassNodes{};
    std::vector<VirtualResource*> mVirtualResources{};
    std::vector<RenderGraphTexture*> mTextures{};
    std::vector<PassNode*>::iterator mActivePassNodesEnd;

    std::vector<ResourceSlot> mResourceSlots{};
    Device& device;


    std::vector<Edge> edges;

    // std::vector<std::unique_ptr<Vi>>
};
