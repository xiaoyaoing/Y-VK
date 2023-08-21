//
// Created by pc on 2023/8/17.
//

#include "Vulkan.h"

#pragma once

class Texture;

namespace VkCommon {
    VkDescriptorPoolSize DescriptorPoolSize(VkDescriptorType type, uint32_t descriptorCount);

    VkDescriptorImageInfo DescriptorImageInfo(Texture &texture);
     void setImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout,
                        VkImageLayout newLayout) ;

};


