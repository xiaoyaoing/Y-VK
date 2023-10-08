#include "DescriptorPool.h"
#include "Device.h"

DescriptorPool::DescriptorPool(Device& device, const DescriptorLayout& layout, uint32_t poolSize)
{
    const auto& bindings = layout.getBindings();

    std::unordered_map<VkDescriptorType, uint32_t> descTypeCounts;
    std::vector<VkDescriptorPoolSize> poolSizes;

    for (auto& binding : bindings)
    {
        descTypeCounts[binding.descriptorType] += binding.descriptorCount;
    }
    
    for (auto& descIt : descTypeCounts)
    {
        poolSizes.push_back({.type = descIt.first, .descriptorCount = descIt.second});
    }

    auto poolInfo = VkDescriptorPoolCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, .maxSets = poolSize,
        .poolSizeCount = toUint32(poolSizes.size()),
        .pPoolSizes = poolSizes.data()
    };
    VK_CHECK_RESULT(vkCreateDescriptorPool(device.getHandle(), &poolInfo, nullptr, &pool))
}

DescriptorPool::DescriptorPool(Device& device, const std::vector<VkDescriptorPoolSize>& poolSizes,
                               const uint32_t maxNumSets)
{
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(maxNumSets);
    VK_CHECK_RESULT(vkCreateDescriptorPool(device.getHandle(), &poolInfo, nullptr, &pool));
}
