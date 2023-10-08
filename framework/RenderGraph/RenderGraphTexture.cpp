#include "RenderGraphTexture.h"

#include "Common/ResourceCache.h"
#include "Images/Image.h"

// const HwTexture& RenderGraphTexture::getHandle() const
// {
//     return hwTexture;
// }

sg::SgImage *RenderGraphTexture::getHwTexture() const {
    return hwTexture;
}

void RenderGraphTexture::create(const char *name, const Descriptor &descriptor) {
    hwTexture = &ResourceCache::getResourceCache().requestSgImage(
            name, {descriptor.extent.width, descriptor.extent.height, 1},
            descriptor.format, descriptor.usageFlags, descriptor.memoryUsage, VK_IMAGE_VIEW_TYPE_2D);
}

void RenderGraphTexture::setHwTexture(sg::SgImage *hwTexture) {
    this->hwTexture = hwTexture;
}
