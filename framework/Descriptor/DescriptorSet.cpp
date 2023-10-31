#include "DescriptorSet.h"
#include "DescriptorLayout.h"
#include "DescriptorPool.h"
#include "Buffer.h"
#include "Device.h"

DescriptorSet::DescriptorSet(Device& device,
                             DescriptorPool& descriptorPool,
                             DescriptorLayout& descriptorSetLayout,
                             const uint32_t descSetCount) : _device(device)
{
    // const VkDescriptorSetLayout setLayout = descriptorSetLayout.getHandle();
    // VkDescriptorSetAllocateInfo descSetAllocInfo{};
    // descSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    // descSetAllocInfo.pNext = nullptr;
    // descSetAllocInfo.descriptorPool = descriptorPool.getHandle();
    // descSetAllocInfo.descriptorSetCount = descSetCount;
    // descSetAllocInfo.pSetLayouts = &setLayout;
    // VK_CHECK_RESULT(vkAllocateDescriptorSets(device.getHandle(), &descSetAllocInfo, &_descriptorSet));
}

void DescriptorSet::updateBuffer(const std::vector<Buffer*>& buffers, const uint32_t dstBinding,
                                 const uint32_t descCount, VkDescriptorType descType)
{
    std::vector<VkDescriptorBufferInfo> bufferInfos;
    bufferInfos.reserve(buffers.size());
    for (size_t i = 0; i < buffers.size(); ++i)
    {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = buffers[i]->getHandle();
        bufferInfo.offset = 0;
        bufferInfo.range = buffers[i]->getSize();
        bufferInfos.emplace_back(std::move(bufferInfo));
    }

    VkWriteDescriptorSet writeSet = {};
    writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeSet.pNext = nullptr;
    writeSet.dstBinding = dstBinding;
    writeSet.dstSet = _descriptorSet;
    writeSet.descriptorCount = descCount;
    writeSet.descriptorType = descType;
    writeSet.pBufferInfo = bufferInfos.data();

    vkUpdateDescriptorSets(_device.getHandle(), 1, &writeSet, 0, nullptr);
}

void DescriptorSet::updateImage(const std::vector<VkDescriptorImageInfo>& imageInfos,
                                const uint32_t dstBinding,
                                VkDescriptorType descType)
{
    VkWriteDescriptorSet writeSet = {};
    writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeSet.pNext = nullptr;
    writeSet.dstBinding = dstBinding;
    writeSet.dstSet = _descriptorSet;
    writeSet.descriptorCount = static_cast<uint32_t>(imageInfos.size());
    writeSet.descriptorType = descType;
    writeSet.pImageInfo = imageInfos.data();

    vkUpdateDescriptorSets(_device.getHandle(), 1, &writeSet, 0, nullptr);
}

DescriptorSet::~DescriptorSet()
{
}

DescriptorSet::DescriptorSet(Device& device, const DescriptorLayout& descriptorSetLayout,
                             DescriptorPool& descriptorPool, const BindingMap<VkDescriptorBufferInfo>& bufferInfos,
                             const BindingMap<VkDescriptorImageInfo>& imageInfos) : _device(device)
{
    const VkDescriptorSetLayout setLayout = descriptorSetLayout.getHandle();
    // VkDescriptorSetAllocateInfo descSetAllocInfo{};
    // descSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    // descSetAllocInfo.pNext = nullptr;
    // descSetAllocInfo.descriptorPool = descriptorPool.getHandle();
    // descSetAllocInfo.descriptorSetCount = 1;
    // descSetAllocInfo.pSetLayouts = &setLayout;
    // VK_CHECK_RESULT(vkAllocateDescriptorSets(device.getHandle(), &descSetAllocInfo, &_descriptorSet))

   _descriptorSet = descriptorPool.allocate();

    std::vector<VkWriteDescriptorSet> writeSets;
    for (auto& bufferIt : bufferInfos)
    {
        auto bindingIndex = bufferIt.first;
        auto& bufferBindings = bufferIt.second;

        auto& bindingInfo = descriptorSetLayout.getLayoutBindingInfo(bindingIndex);


        for (auto& elementIt : bufferBindings)
        {
            auto& bufferInfo = elementIt.second;

            VkWriteDescriptorSet writeSet{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = _descriptorSet,
                .dstBinding = bindingIndex,
                .dstArrayElement = elementIt.first,
                .descriptorCount = 1,
                .descriptorType = bindingInfo.descriptorType,
                .pBufferInfo = &bufferInfo,
            };

            writeSets.push_back(writeSet);
        }
    }

    for (auto& imageIt : imageInfos)
    {
        auto bindingIndex = imageIt.first;
        auto& imageBindings = imageIt.second;

        auto& bindingInfo = descriptorSetLayout.getLayoutBindingInfo(bindingIndex);


        for (auto& elementIt : imageBindings)
        {
            auto& imageInfo = elementIt.second;

            VkWriteDescriptorSet writeSet{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = _descriptorSet,
                .dstBinding = bindingIndex,
                .dstArrayElement = elementIt.first,
                .descriptorCount = 1,
                .descriptorType = bindingInfo.descriptorType,
                .pImageInfo = &imageInfo,
            };

            writeSets.push_back(writeSet);
        }
    }

    vkUpdateDescriptorSets(device.getHandle(),
                           toUint32(writeSets.size()),
                           writeSets.data(),
                           0,
                           nullptr);
}
