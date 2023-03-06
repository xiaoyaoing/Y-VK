#include "DescriptorPool.h"
#include <Device.h>

DescriptorPool::DescriptorPool(ptr<Device> device, const std::vector<uint32_t> &sizes, const uint32_t maxNumSets,
                               VkDescriptorPoolCreateFlags flags) {

    auto pollSizes = AllocateVector<VkDescriptorPoolSize>(sizes.size());
    for (auto size: sizes) {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount =
                static_cast<uint32_t>(size);
        pollSizes.push_back(poolSize);
    }
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = sizes.size();
    poolInfo.pPoolSizes = pollSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(maxNumSets);
    ASSERTEQ(vkCreateDescriptorPool(device->getHandle(), &poolInfo, nullptr, &pool), VK_SUCCESS);
}
