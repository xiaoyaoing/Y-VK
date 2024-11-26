#include "RenderGraphTexture.h"

#include "Common/ResourceCache.h"
#include "Core/Images/Image.h"

#include "Core/Images/ImageUtil.h"

// const HwTexture& RenderGraphTexture::getHandle() const
// {
//     return hwTexture;
// }

SgImage* RenderGraphTexture::getHwTexture() const {
    return mHwTexture;
}

void RenderGraphTexture::devirtualize() {
    if (imported)
        return;
    if (mHwTexture != nullptr) {
        LOGE("Texture already devirtualized")
    }

    VkFormat format = mDescriptor.format == VK_FORMAT_UNDEFINED ? ImageUtil::getFormat(mDescriptor.useage) : mDescriptor.format;

    //2024 1 20 all renderGraph texture created with transfer_src usage for debug
    mHwTexture = &ResourceCache::getResourceCache().requestSgImage(
        mName, {mDescriptor.extent.width, mDescriptor.extent.height, 1}, format, ImageUtil::getUsageFlags(mDescriptor.useage | TextureUsage::TRANSFER_SRC), VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_VIEW_TYPE_2D);
}

void RenderGraphTexture::destroy() {
    //todo handle this
    delete this;
}

#include <vulkan/vulkan.h>
#include <string>

std::string ImageLayoutToString(VkImageLayout layout) {
    switch (layout) {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            return "VK_IMAGE_LAYOUT_UNDEFINED";
        case VK_IMAGE_LAYOUT_GENERAL:
            return "VK_IMAGE_LAYOUT_GENERAL";
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            return "VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL";
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            return "VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL";
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
            return "VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL";
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            return "VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL";
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            return "VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL";
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            return "VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL";
        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            return "VK_IMAGE_LAYOUT_PREINITIALIZED";
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
            return "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL";
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
            return "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL";
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            return "VK_IMAGE_LAYOUT_PRESENT_SRC_KHR";
        case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
            return "VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR";
        case VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV:
            return "VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV";
        case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
            return "VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT";
        default:
            return "UNKNOWN_LAYOUT";
    }
}

RenderResourceType RenderGraphTexture::getType() const {
    return RenderResourceType::ETexture;
}
unsigned short RenderGraphTexture::defaultUsage() const {
    return 0;
}
void RenderGraphTexture::resloveUsage(ResourceBarrierInfo& barrierInfo, uint16_t lastUsage, uint16_t nextUsage, RenderPassType lastPassType, RenderPassType nextPassType) {
    auto oldLayout = ImageUtil::getDefaultLayout(static_cast<TextureUsage>(lastUsage));
    auto newLayout = ImageUtil::getDefaultLayout(static_cast<TextureUsage>(nextUsage));

    auto [srcAccessMask, dstAccessMask, srcStage, dstStage, vkOldLayout, vkNewLayout] = ImageUtil::getVkTransition(oldLayout, newLayout);
    srcStage = ImageUtil::getStageFlags(lastPassType);
    dstStage = ImageUtil::getStageFlags(nextPassType);

    auto & imageBarrier = barrierInfo.imageBarriers.emplace_back();
    imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    imageBarrier.srcStageMask = srcStage;
    imageBarrier.dstStageMask = dstStage;
    imageBarrier.srcAccessMask = srcAccessMask;
    imageBarrier.dstAccessMask = dstAccessMask;
    imageBarrier.oldLayout = vkOldLayout;
    imageBarrier.newLayout = vkNewLayout;
    imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.image = mHwTexture->getVkImage().getHandle();
    imageBarrier.subresourceRange = getHwTexture()->getVkImageView().getSubResourceRange();

    LOGI("Image name: {}, old layout: {}, new layout: {}", mName.c_str(), ImageLayoutToString(vkOldLayout).c_str(), ImageLayoutToString(vkNewLayout).c_str());
    mHwTexture->getVkImage().setLayout(newLayout);
}


// RenderGraphTexture::~RenderGraphTexture() {
//     // if (!imported)
//     //     delete mHwTexture;
// }

RenderGraphTexture::~RenderGraphTexture() {
}
RenderGraphTexture::RenderGraphTexture(const std::string& name, SgImage* hwTexture) : mHwTexture(hwTexture), imported(true),
                                                                                      mDescriptor({}) {
    RenderGraphNode::setName(name);
}

RenderGraphTexture::RenderGraphTexture(const std::string& name, const Descriptor& descriptor) : mDescriptor({descriptor}) {
    RenderGraphNode::setName(name);
}

bool RenderGraphTexture::isDepthStencilTexture() const {
    if (imported)
        return isDepthOrStencilFormat(getHwTexture()->getFormat());
    return isDepthOrStencilFormat(ImageUtil::getFormat(mDescriptor.useage));
}

void RenderGraphTexture::setHwTexture(SgImage* hwTexture) {
    this->mHwTexture = hwTexture;
}