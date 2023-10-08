#include "RenderGraph.h"
#include <CommandBuffer.h>

void RenderGraph::Builder::read(VirtualResource* resource, PassNode* node)
{
}

void RenderGraph::Builder::write(VirtualResource* resource, PassNode* node)
{
}

void RenderGraph::Builder::declare(const char* name, const RenderGraphPassDescriptor& desc)
{
    auto rNode = static_cast<RenderPassNode*>(node);
    rNode->renderTargetData.desc = desc;
}

RenderGraph::RenderGraph(Device& device) : device(device)
{
}

RenderGraphHandle RenderGraph::addResource(VirtualResource* resource)
{
    const RenderGraphHandle handle(virtualResources.size());

    auto& slot = resourceSlots.emplace_back();
    slot.rid = virtualResources.size();
    virtualResources.push_back(resource);

    return handle;
    //return RenderGraphHandle(RenderGraphHandle());
}

void RenderGraph::setUp()
{
}

VirtualResource* RenderGraph::getResource(RenderGraphHandle handle)
{
    auto& slot = resourceSlots[handle.index];
    return virtualResources[slot.rid];
    // return nullptr;
}

void RenderGraph::compile()
{
}

Device& RenderGraph::getDevice()
{
    return device;
}

void RenderGraph::execute(CommandBuffer& commandBuffer)
{
    {
        //todo handle compile

        for (auto& resource : virtualResources)
        {
            resource->create();
        }

        //   commandBuffer.beginRecord(0);

        for (auto& node : renderGraphNodes)
        {
            node->execute(*this, commandBuffer);
        }
    }
}


RenderGraphId<RenderGraphTexture> RenderGraph::import(const char *name, sg::SgImage *hwTexture) {
    RenderGraphTexture texture;
    texture.setHwTexture(hwTexture);
    VirtualResource * resource = new ImportedResource<RenderGraphTexture>(name,texture);
    return RenderGraphId<RenderGraphTexture>(addResource(resource));
}