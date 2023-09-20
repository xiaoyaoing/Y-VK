#pragma once
#include "PassNode.h"
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

class VirtualResource
{
    virtual void create();
    virtual void destroy();
};

template <class RESOURCE>
class Resource : public VirtualResource
{
public:
    Resource(const char* name, typename RESOURCE::Descriptor desc);

    RESOURCE resource{};
};


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
    RenderGraphId<RESOURCE> create(char* name, const typename RESOURCE::Descriptor& desc) noexcept
    {
        VirtualResource* virtualResource = Resource<RESOURCE>(name, desc);
        return RenderGraphId<RESOURCE>(addResource(virtualResource));
    }

    class Builder
    {
    public:
        void read(VirtualResource* resource, PassNode* node);
        void write(VirtualResource* resource, PassNode* node);
        void declare(const char* name,
                     const RenderGraphPassDescriptor& desc);
        // void read(VirtualResource * resource,PassNode * node);


    protected:
        PassNode* node;
    };

    template <typename RESOURCE>
    RenderGraphId<RESOURCE> create(const char* name, const typename RESOURCE::Descriptor& desc = {})
    {
        VirtualResource* virtualResource = new Resource<RESOURCE>(name, desc);
        return RenderGraphId<RESOURCE>(addResource(virtualResource));
    }

    RenderGraphHandle addResource(VirtualResource* resource);

    void setUp();

    void execute(CommandBuffer& commandBuffer)
    {
        for (auto node : renderGraphNodes)
        {
            node->execute(*this, commandBuffer);
        }
    }


    void addPassInternal(const char* name, RenderGraphPassBase* base)
    {
        auto node = new PassNode(*this, name, base);
        renderGraphNodes.push_back(node);
    }


    template <typename Data, typename Setup, typename Execute>
    void addPass(const char* name, Setup setup, Execute&& execute)
    {
        auto renderpass = new RenderGraphPass<Data, Execute> > (std::forward<Execute>(execute));
        setup(renderpass->getData());
    }

    VirtualResource* getResource(RenderGraphHandle handle);

    void compile();

    Device& getDevice();

private:
    std::vector<PassNode*> renderGraphNodes;
    std::vector<VirtualResource*> virtualResources;
    Device& device;
    // std::vector<std::unique_ptr<Vi>>
};
