#pragma once

#include "PassNode.h"
#include "VirtualResource.h"
#include "RenderTarget.h"
#include "RenderGraphId.h"
#include "RenderGraphPass.h"
#include "RenderGraphTexture.h"
#include "RenderPass.h"
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
//         RenderGraphId<RenderGraphTexture> array[ATTACHMENT_COUNT] = {};
//         struct {
//             RenderGraphId<RenderGraphTexture> color[backend::MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT];
//             RenderGraphId<RenderGraphTexture> depth;
//             RenderGraphId<RenderGraphTexture> stencil;
//         };
//     };
struct RenderPassData
{
    VkViewport viewport{};
    VkClearValue clearColor{};
    uint8_t samples = 0; // # of samples (0 = unset, default)
    //  backend::TargetBufferFlags clearFlags{};
    //  backend::TargetBufferFlags discardStart{};
};


class RenderGraph
{
public:
    RenderGraph(Device& device);


    template <typename RESOURCE>
    RenderGraphId<RESOURCE> create(char* name, const typename RESOURCE::Descriptor& desc)
    {
        VirtualResource* virtualResource = Resource<RESOURCE>(name, desc);
        return RenderGraphId<RESOURCE>(addResource(virtualResource));
    }

    RenderGraphId<RenderGraphTexture> import(const char * name,sg::SgImage * hwTexture);

    class Builder
    {
    public:
        void read(VirtualResource* resource, PassNode* node);

        void write(VirtualResource* resource, PassNode* node);

        void declare(const char* name,
                     const RenderGraphPassDescriptor& desc);

        Builder(PassNode* node, RenderGraph& renderGraph)
            : node(node),
              renderGraph(renderGraph)
        {
        }

    protected:
        PassNode* node;
        RenderGraph& renderGraph;
    };

    template <typename RESOURCE>
    RenderGraphId<RESOURCE> create(const char* name, const typename RESOURCE::Descriptor& desc = {})
    {
        VirtualResource* virtualResource = new Resource<RESOURCE>(name, desc);
        return RenderGraphId<RESOURCE>(addResource(virtualResource));
    }



    RenderGraphHandle addResource(VirtualResource* resource);

    void setUp();

    void execute(CommandBuffer& commandBuffer);


    PassNode* addPassInternal(const char* name, RenderGraphPassBase* base)
    {
        //auto node = ;
        auto node = new RenderPassNode(*this, name, base);
        renderGraphNodes.emplace_back(node);
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

    VirtualResource* getResource(RenderGraphHandle handle);

    void compile();

    Device& getDevice();

    ~RenderGraph()
    {
        for (const auto& passNode : renderGraphNodes)
            delete passNode;
        for (const auto& resource : virtualResources)
            delete resource;
    }

private:
    struct ResourceSlot
    {
        using Version = RenderGraphHandle::Version;
        using Index = int16_t;
        Index rid = 0; // VirtualResource* index in mResources
        Index nid = 0; // ResourceNode* index in mResourceNodes
        Index sid = -1; // ResourceNode* index in mResourceNodes for reading subresource's parent
        Version version = 0;
    };

    std::vector<PassNode*> renderGraphNodes{};
    std::vector<VirtualResource*> virtualResources{};
    //  std::vector<ResourceNode *> mResourceNodes;
    std::vector<ResourceSlot> resourceSlots{};
    Device& device;
    // std::vector<std::unique_ptr<Vi>>
};
