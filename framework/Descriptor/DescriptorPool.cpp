#include "DescriptorPool.h"
#include "Device.h"

DescriptorPool::DescriptorPool(Device& device, const DescriptorLayout& layout, uint32_t poolSize): layout(layout),
    device(device), maxPoolSets(poolSize)
{
    const auto& bindings = layout.getBindings();

    std::unordered_map<VkDescriptorType, uint32_t> descTypeCounts;
    //  std::vector<VkDescriptorPoolSize> poolSizes;

    for (const auto& binding : bindings)
    {
        descTypeCounts[binding.descriptorType] += binding.descriptorCount;
    }

    for (const auto& descIt : descTypeCounts)
    {
        poolSizes.push_back({.type = descIt.first, .descriptorCount = descIt.second});
    }

    // auto poolInfo = VkDescriptorPoolCreateInfo{
    //     .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, .maxSets = poolSize,
    //     .poolSizeCount = toUint32(poolSizes.size()),
    //     .pPoolSizes = poolSizes.data()
    // };
    // VK_CHECK_RESULT(vkCreateDescriptorPool(device.getHandle(), &poolInfo, nullptr, &pool))
}

DescriptorPool::DescriptorPool(Device& device, const std::vector<VkDescriptorPoolSize>& poolSizes,
                               const uint32_t maxNumSets): layout(layout), device(device), maxPoolSets(maxNumSets)
{
    // VkDescriptorPoolCreateInfo poolInfo{};
    // poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    // poolInfo.poolSizeCount = poolSizes.size();
    // poolInfo.pPoolSizes = poolSizes.data();
    // poolInfo.maxSets = static_cast<uint32_t>(maxNumSets);
    // VK_CHECK_RESULT(vkCreateDescriptorPool(device.getHandle(), &poolInfo, nullptr, &pool));
}

VkDescriptorSet DescriptorPool::allocate()
{
    if (pools.size() <= curPoolIdx || poolCounts[curPoolIdx - 1] < maxPoolSets)
    {
        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = poolSizes.size();
        poolInfo.pPoolSizes = poolSizes.data();

        poolInfo.maxSets = maxPoolSets;
        pools.push_back(VK_NULL_HANDLE);
        VK_CHECK_RESULT(vkCreateDescriptorPool(device.getHandle(), &poolInfo, nullptr, &pools[curPoolIdx++]))
    }

    VkDescriptorSetLayout setLayout = layout.getHandle();

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pools[curPoolIdx - 1];
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &setLayout;

    poolCounts.push_back(0);

    VkDescriptorSet descriptor;
    VK_CHECK_RESULT(vkAllocateDescriptorSets(device.getHandle(), &allocInfo, &descriptor))
    return descriptor;
}

const DescriptorLayout& DescriptorPool::getDescriptorLayout() const
{
    return layout;
}
