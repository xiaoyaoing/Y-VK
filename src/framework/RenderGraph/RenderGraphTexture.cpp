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

void RenderGraphTexture::create(const char* name, const Descriptor& descriptor) {
    mHwTexture = &ResourceCache::getResourceCache().requestSgImage(
        name, {descriptor.extent.width, descriptor.extent.height, 1}, ImageUtil::getFormat(descriptor.useage), ImageUtil::getUsageFlags(descriptor.useage), VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_VIEW_TYPE_2D);
}

void RenderGraphTexture::devirtualize() {
    if (imported)
        return;
    if (mHwTexture != nullptr) {
        LOGE("Texture already devirtualized")
    }
    //2024 1 20 all renderGraph texture created with transfer_src usage for debug
    mHwTexture = &ResourceCache::getResourceCache().requestSgImage(
        mName, {mDescriptor.extent.width, mDescriptor.extent.height, 1}, ImageUtil::getFormat(mDescriptor.useage), ImageUtil::getUsageFlags(mDescriptor.useage | TextureUsage::TRANSFER_SRC), VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_VIEW_TYPE_2D);
}

void RenderGraphTexture::destroy() {
    //todo handle this
    delete this;
}

RENDER_GRAPH_RESOURCE_TYPE RenderGraphTexture::getType() const {
    return RENDER_GRAPH_RESOURCE_TYPE::ETexture;
}

void RenderGraphTexture::resloveUsage(CommandBuffer& commandBuffer, uint16_t usage) {
    const auto  newLayout        = ImageUtil::getDefaultLayout(static_cast<TextureUsage>(usage));
    const auto& subResourceRange = getHwTexture()->getVkImageView().getSubResourceRange();
    mHwTexture->getVkImage().transitionLayout(commandBuffer, newLayout, subResourceRange);
}

// RenderGraphTexture::~RenderGraphTexture() {
//     // if (!imported)
//     //     delete mHwTexture;
// }

RenderGraphTexture::RenderGraphTexture(const char* name, SgImage* hwTexture) : mHwTexture(hwTexture), imported(true),
                                                                               mDescriptor({}) {
    RenderGraphNode::setName(name);
}

RenderGraphTexture::RenderGraphTexture(const char* name, const Descriptor& descriptor) : mDescriptor({descriptor}) {
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