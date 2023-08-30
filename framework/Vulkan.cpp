//
// Created by pc on 2023/8/5.
//
#include <Vulkan.h>

namespace Default {
    std::vector<VkClearValue> clearValues() {
        std::vector<VkClearValue> clearValues(2);
        clearValues[0].color = {{0.f, 0.f, 0.f, 0.0f}};
        clearValues[1].depthStencil = {1.0f, 0};
        return clearValues;
    }
}