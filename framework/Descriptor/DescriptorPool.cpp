#include "DescriptorPool.h"
#include "Device.h"

DescriptorPool::DescriptorPool(ptr<Device> device, const std::vector<VkDescriptorPoolSize> &poolSizes, const uint32_t maxNumSets,
                               VkDescriptorPoolCreateFlags flags) {


    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(maxNumSets);
    ASSERT(vkCreateDescriptorPool(device->getHandle(), &poolInfo, nullptr, &pool)==VK_SUCCESS,"create descriptor pool")
}
