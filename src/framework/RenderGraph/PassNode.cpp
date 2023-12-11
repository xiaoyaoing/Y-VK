#include "PassNode.h"

#include "Core/Device/Device.h"
#include <Common/ResourceCache.h>

#include "Core/RenderContext.h"
#include "VirtualResource.h"
#include "RenderGraph/RenderGraph.h"
#include "Core/Images/ImageUtil.h"


void RenderPassNode::RenderPassData::devirtualize(RenderGraph &renderGraph, const RenderPassNode &node) {
    std::vector<SgImage *> images;
    std::vector<Attachment> attachments;
    for (const auto &color: desc.textures) {
        auto texture = renderGraph.getTexture(color);
        auto hwTexture = texture->getHwTexture();

        auto undefined = texture->first == &node && !texture->imported;
        auto write = renderGraph.isWrite(color, &node);
        auto read = renderGraph.isRead(color, &node);


        auto attachment = Attachment{
                .format = hwTexture->getFormat(),
                .samples = hwTexture->getVkImage().getSampleCount(),
                .usage = hwTexture->getVkImage().getUseFlags(),
                .initial_layout = hwTexture->getVkImage().getLayout(hwTexture->getVkImageView().getSubResourceRange()),
                //todo fix this
                .loadOp = desc.textures.size() == 1
                          ? VK_ATTACHMENT_LOAD_OP_LOAD
                          : ((undefined | write) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD),

                .storeOp = VK_ATTACHMENT_STORE_OP_STORE
        };

        images.push_back(hwTexture);
        attachments.push_back(attachment);
    }
    renderTarget = std::make_unique<RenderTarget>(images, attachments);
}

RenderTarget &RenderPassNode::RenderPassData::getRenderTarget() {
    return *renderTarget;
}

VkPipeline pipeline = VK_NULL_HANDLE;


void RenderPassNode::execute(RenderGraph &renderGraph, CommandBuffer &commandBuffer) {
    renderPassData.devirtualize(renderGraph, *this);

    auto &renderTarget = renderPassData.getRenderTarget();

    std::vector<SubpassInfo> subpassInfos;

    {
        auto &renderGraphSubpassInfos = renderPassData.desc.subpasses;

        //  subpassInfos.resize(renderPassData.desc.getSubpassCount());

        std::unordered_map<RenderGraphHandle, size_t, RenderGraphHandle::Hash> attachmentMap;

        for (size_t i = 0; i < renderPassData.desc.textures.size(); i++) {
            attachmentMap.insert_or_assign(renderPassData.desc.textures[i], i);
        }


        std::ranges::transform(renderGraphSubpassInfos.begin(), renderGraphSubpassInfos.end(),
                               std::back_inserter(subpassInfos), [&](const auto &renderGraphSubassInfo) {
                    SubpassInfo subpassInfo{};
                    subpassInfo.disableDepthStencilAttachment = renderGraphSubassInfo.disableDepthTest;
                    subpassInfo.inputAttachments.resize(renderGraphSubassInfo.inputAttachments.size());
                    subpassInfo.outputAttachments.resize(renderGraphSubassInfo.outputAttachments.size());
                    std::ranges::transform(renderGraphSubassInfo.inputAttachments.begin(),
                                           renderGraphSubassInfo.inputAttachments.end(),
                                           subpassInfo.inputAttachments.begin(), [&](const auto &handle) {
                                return attachmentMap[handle];
                            });
                    std::ranges::transform(renderGraphSubassInfo.outputAttachments.begin(),
                                           renderGraphSubassInfo.outputAttachments.end(),
                                           subpassInfo.outputAttachments.begin(), [&](const auto &handle) {
                                return attachmentMap[handle];
                            });
                    return subpassInfo;
                });
    }


    auto hwTextures = renderTarget.getHwTextures();
    RenderContext::g_context->getPipelineState().setPipelineType(PIPELINE_TYPE::E_GRAPHICS);
    RenderContext::g_context->beginRenderPass(commandBuffer, renderTarget, subpassInfos);

    RenderPassContext context = {
            .commandBuffer = RenderContext::g_context->getGraphicBuffer()
    };
    mRenderPass->execute(context);

    RenderContext::g_context->endRenderPass(commandBuffer, renderTarget);
}

void RenderPassNode::declareRenderTarget(const char *name, const RenderGraphPassDescriptor &descriptor) {
    renderPassData.name = name;

    renderPassData.desc = descriptor;
}

RenderPassNode::RenderPassNode(RenderGraph &renderGraph, const char *name, RenderGraphPassBase *base) : mRenderPass(
        base), name(name) {
}

void RenderPassNode::declareRenderPass(const char *name, const RenderGraphPassDescriptor &descriptor) {
    renderPassData.desc = descriptor;
}


void PassNode::resolveTextureUsages(RenderGraph &renderGraph, CommandBuffer &commandBuffer) {
    for (auto &textureIt: resourceUsages) {
        textureIt.first->resloveUsage(commandBuffer, textureIt.second);
        // const auto *texture = textureIt.first;
        // const auto newLayout = ImageUtil::getDefaultLayout(textureIt.second);
        // const auto subsubsource = texture->getHwTexture()->getVkImageView().getSubResourceRange();
        // texture->getHwTexture()->getVkImage().transitionLayout(commandBuffer, newLayout, subsubsource);
    }
}

void PassNode::addResourceUsage(ResourceNode* texture, uint16_t usage)
{
    resourceUsages.emplace(texture,usage);
}



void ImageCopyPassNode::execute(RenderGraph &renderGraph, CommandBuffer &commandBuffer) {

    auto & srcVkImage =renderGraph.getTexture(src)->getHwTexture()->getVkImage();
    auto & dstVkImage =renderGraph.getTexture(dst)->getHwTexture()->getVkImage();
    
    VkImageCopy copy_region{};
    copy_region.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1    };
    copy_region.srcOffset      = {0, 0, 0};
    copy_region.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.dstOffset      = {0, 0, 0};
    copy_region.extent         = {srcVkImage.getExtent().width,srcVkImage.getExtent().height, 1};

    vkCmdCopyImage(commandBuffer.getHandle(),srcVkImage.getHandle(),VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    dstVkImage.getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
}

ImageCopyPassNode::ImageCopyPassNode(RenderGraphHandle src, RenderGraphHandle dst):src(src),dst(dst)
{
}

void ComputePassNode::execute(RenderGraph& renderGraph, CommandBuffer& commandBuffer)
{    RenderContext::g_context->getPipelineState().setPipelineType(PIPELINE_TYPE::E_COMPUTE);

    RenderPassContext context{            .commandBuffer = RenderContext::g_context->getGraphicBuffer()};
    mPass->execute(context);
}

void RayTracingPassNode::execute(RenderGraph& renderGraph, CommandBuffer& commandBuffer)
{   
    auto &  settings = mPass->getData();
    RenderContext::g_context->getPipelineState().setPipelineType(PIPELINE_TYPE::E_RAY_TRACING)
    .setPipelineLayout(*settings.pipelineLayout).setrTPipelineSettings(settings.rTPipelineSettings);
    RenderContext::g_context->bindAcceleration(0,*settings.accel,0,0);
    RenderPassContext context{.commandBuffer = RenderContext::g_context->getComputeCommandBuffer()};
    mPass->execute(context);
    // RenderContext::g_context->getPipelineState().setPipelineType(PIPELINE_TYPE::E_RAY_TRACING).setPipelineLayout(*layout).setrTPipelineSettings({.maxDepth = 5,.dims = {width,height,1}});
    // renderContext->getPipelineState().setPipelineType(PIPELINE_TYPE::E_RAY_TRACING).setPipelineLayout(*layout).setrTPipelineSettings({.maxDepth = 5,.dims = {width,height,1}});

}


ComputePassNode::ComputePassNode(RenderGraph& renderGraph, const char* name, ComputeRenderGraphPass* base):mPass(base)
{
}

RayTracingPassNode::RayTracingPassNode(RenderGraph& renderGraph, const char* name, RaytracingRenderGraphPass *base):mPass(base)
{
}


