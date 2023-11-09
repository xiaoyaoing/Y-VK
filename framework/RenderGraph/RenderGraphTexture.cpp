#include "RenderGraphTexture.h"

#include "Common/ResourceCache.h"
#include "Images/Image.h"
#include "Images/ImageUtil.h"

// const HwTexture& RenderGraphTexture::getHandle() const
// {
//     return hwTexture;
// }

sg::SgImage* RenderGraphTexture::getHwTexture() const
{
    return mHwTexture;
}

void RenderGraphTexture::create(const char* name, const Descriptor& descriptor)
{
    mHwTexture = &ResourceCache::getResourceCache().requestSgImage(
        name, {descriptor.extent.width, descriptor.extent.height, 1},
        ImageUtil::getFormat(descriptor.useage), ImageUtil::getUsageFlags(descriptor.useage), VMA_MEMORY_USAGE_GPU_ONLY,
        VK_IMAGE_VIEW_TYPE_2D);
}

void RenderGraphTexture::devirtualize()
{
    if (imported)
        return;
    if (mHwTexture != nullptr)
    {
        LOGE("Texture already devirtualized")
    }
    mHwTexture = &ResourceCache::getResourceCache().requestSgImage(
        mName, {mDescriptor.extent.width, mDescriptor.extent.height, 1},
        ImageUtil::getFormat(mDescriptor.useage), ImageUtil::getUsageFlags(mDescriptor.useage),
        VMA_MEMORY_USAGE_GPU_ONLY,
        VK_IMAGE_VIEW_TYPE_2D);
}

void RenderGraphTexture::destroy()
{
    //todo handle this
    delete this;
}

void RenderGraphTexture::resolveTextureUsage(CommandBuffer& commandBuffer)
{
    const auto newLayout = ImageUtil::getDefaultLayout(usage);
    const auto& subResourceRange = getHwTexture()->getVkImageView().getSubResourceRange();
    mHwTexture->getVkImage().transitionLayout(commandBuffer, newLayout, subResourceRange);
}

RenderGraphTexture::RenderGraphTexture(const char* name, sg::SgImage* hwTexture): mHwTexture(hwTexture), imported(true),
    mName(name), mDescriptor({})
{
}

RenderGraphTexture::RenderGraphTexture(const char* name, const Descriptor& descriptor): mName(name),
    mDescriptor({descriptor})
{
}

void RenderGraphTexture::setHwTexture(sg::SgImage* hwTexture)
{
    this->mHwTexture = hwTexture;
}