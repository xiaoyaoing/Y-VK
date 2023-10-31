#pragma once

#include "Vulkan.h"

class DescriptorLayout;
class Device;

class DescriptorPool
{
public:
    static constexpr uint32_t MAX_SETS_PER_POOL = 16;
    DescriptorPool(Device& device, const DescriptorLayout& layout, uint32_t poolSize = MAX_SETS_PER_POOL);
    DescriptorPool(Device& device, const std::vector<VkDescriptorPoolSize>& poolSizes, const uint32_t maxNumSets);
    VkDescriptorSet allocate();
    const DescriptorLayout& getDescriptorLayout() const;
private:
    const DescriptorLayout& layout;
    Device& device;

    std::vector<uint32_t> poolCounts;
    std::vector<VkDescriptorPoolSize> poolSizes;

    std::vector<VkDescriptorPool> pools;
    uint32_t maxPoolSets;
    uint32_t curPoolIdx{0};
};
