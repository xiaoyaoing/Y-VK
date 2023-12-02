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
        auto texture = renderGraph.getResource(color);
        auto hwTexture = texture->getHwTexture();

        auto undefined = texture->first == &node && !texture->imported;
        auto write = renderGraph.isWrite(color, &node);
        auto read = renderGraph.isRead(color, &node);


        auto attachment = Attachment{
                .format = hwTexture->getFormat(),
                .samples = hwTexture->getVkImage().getSampleCount(),
                .usage = hwTexture->getVkImage().getUseFlags(),
                .initial_layout = ImageUtil::getVkImageLayout(
                        hwTexture->getVkImage().getLayout(hwTexture->getVkImageView().getSubResourceRange())),
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
                    SubpassInfo subpassInfo;
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

    RenderContext::g_context->beginRenderPass(commandBuffer, renderTarget, subpassInfos);

    RenderPassContext context = {
            .commandBuffer = commandBuffer,
            .renderTarget = renderTarget
    };
    mRenderPass->execute(context);
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
    for (auto &textureIt: textureUsages) {
        const auto *pResource = textureIt.first;
        const auto newLayout = ImageUtil::getDefaultLayout(textureIt.second);
        const auto subsubsource = pResource->getHwTexture()->getVkImageView().getSubResourceRange();
        pResource->getHwTexture()->getVkImage().transitionLayout(commandBuffer, newLayout, subsubsource);
    }
}

void PassNode::addTextureUsage(const RenderGraphTexture *texture, RenderGraphTexture::Usage usage) {
    if (textureUsages.contains(texture))
        textureUsages[texture] = textureUsages[texture] | usage;
    else
        textureUsages[texture] = usage;
}

void PresentPassNode::execute(RenderGraph &renderGraph, CommandBuffer &commandBuffer) {
}
