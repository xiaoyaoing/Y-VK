#include "PassNode.h"

#include "Device.h"
#include <Common/ResourceCache.h>


void RenderPassNode::RenderPassData::devirtualize(RenderGraph& renderGraph)
{
    std::vector<Image> images;
    std::vector<VulkanAttachment> attachments;
    for (size_t i = 0; i < desc.color.size(); i++)
    {
        const auto* pResource = static_cast<const Resource<RenderGraphTexture>*>(renderGraph.
            getResource(desc.color[i]));
        auto hwTexture = pResource->resource.getHandle();
        //images.emplace_back(hwTexture->image);
    }
}

void RenderPassNode::execute(RenderGraph& renderGraph, CommandBuffer& commandBuffer)
{
    std::vector<Attachment> attachments{};


    auto& renderPass = renderGraph.getDevice().getResourceCache().requestRenderPass(attachments, {});

    auto& framebuffer = ResourceCache::getResourceCache().requestFrameBuffer(
        renderTargetData.getRenderTarget(), renderPass);

    commandBuffer.beginRenderPass(renderPass, framebuffer, Default::clearValues(), {});

    RenderPassContext context = {.renderPass = renderPass, .commandBuffer = commandBuffer};

    mRenderPass->execute(context);
}

void RenderPassNode::declareRenderTarget(const char* name, const RenderGraphPassDescriptor& descriptor)
{
    renderTargetData.name = name;

    renderTargetData.desc = descriptor;
}
