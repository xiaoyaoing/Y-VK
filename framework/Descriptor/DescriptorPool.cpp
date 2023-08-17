#include "DescriptorPool.h"
#include "Device.h"

DescriptorPool::DescriptorPool(Device &device, const std::vector<VkDescriptorPoolSize> &poolSizes,
                               const uint32_t maxNumSets) {


    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(maxNumSets);
    VK_CHECK_RESULT(vkCreateDescriptorPool(device.getHandle(), &poolInfo, nullptr, &pool));

}
