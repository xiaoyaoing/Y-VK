//
// Created by pc on 2023/8/17.
//

#include "VulkanInitializers.h"
#include <Texture.h>
namespace VulkanInitializers {
    VkDescriptorPoolSize DescriptorPoolSize(VkDescriptorType type, uint32_t descriptorCount) {
        return {type, descriptorCount};
    }

    VkDescriptorImageInfo DescriptorImageInfo(Texture &texture) {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.sampler = texture.
        return VkDescriptorImageInfo();
    }
}