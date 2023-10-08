#pragma once

#include "Vulkan.h"

class DescriptorLayout;
class Device;

class DescriptorPool
{
public:
    VkDescriptorPool getHandle() const
    {
        return pool;
    }

    static constexpr uint32_t MAX_SETS_PER_POOL = 16;
    DescriptorPool(Device& device, const DescriptorLayout& layout, uint32_t poolSize = MAX_SETS_PER_POOL);
    DescriptorPool(Device& device, const std::vector<VkDescriptorPoolSize>& poolSizes, const uint32_t maxNumSets);

private:
    VkDescriptorPool pool;
};
