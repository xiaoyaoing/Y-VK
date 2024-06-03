#include "VirtualViewport.h"
SgImage& VirtualViewport::getImage(uint32_t index) {
    return *images[index];
}
VirtualViewport::VirtualViewport(Device& device, VkExtent2D extent, uint32_t imageCount) {
    for (uint32_t i = 0; i < imageCount; i++) {
        images.push_back(std::make_unique<SgImage>(device, "virtual viewport" + std::to_string(i), VkExtent3D{extent.width, extent.height, 1}, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_VIEW_TYPE_2D));
    }
}
VkExtent2D VirtualViewport::getExtent() {
    return images[0]->getExtent2D();
}
