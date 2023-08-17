#include "Vulkan.h"

class Device;

class DescriptorPool {
    VkDescriptorPool pool;


public:
    VkDescriptorPool getHandle() {
        return pool;
    }


    DescriptorPool(Device &device, const std::vector<VkDescriptorPoolSize> &poolSizes, const uint32_t maxNumSets);
};