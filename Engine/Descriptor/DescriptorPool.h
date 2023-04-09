#include "Engine/Vulkan.h"
class Device;

class DescriptorPool{
    VkDescriptorPool  pool;


public:
    VkDescriptorPool getHandle(){
        return pool;
    }
    DescriptorPool(ptr<Device> device,const std::vector<uint32_t>& poolSizes);
     DescriptorPool(ptr<Device> device,
                    const std::vector<VkDescriptorPoolSize> &poolSizes,
                    const uint32_t maxNumSets,
                    VkDescriptorPoolCreateFlags flags);
};