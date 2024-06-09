//
// Created by 打工人 on 2023/3/6.
//

#include "CommandPool.h"
#include "Core/Queue.h"
#include "Core/Device/Device.h"

//CommandPool::CommandPool(ptr<Device> device, ptr<Queue> queue, VkCommandPoolCreateFlags flags) : _device(device) {
//    VkCommandPoolCreateInfo info{};
//    info.queueFamilyIndex = queue->getFamilyIndex();
//    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
//    info.flags = flags;
//    info.pNext = nullptr;
//    VK_CHECK_RESULT(vkCreateCommandPool(device->getHandle(), &info, nullptr, &_pool));
//}

CommandPool::CommandPool(Device& device, uint32_t queueFamilyIndex, CommandBuffer::ResetMode resetMode) : _device(
                                                                                                              device) {

    VkCommandPoolCreateFlags flags;
    switch (resetMode) {
        case CommandBuffer::ResetMode::ResetIndividually:
        case CommandBuffer::ResetMode::AlwaysAllocate:
            flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            break;
        case CommandBuffer::ResetMode::ResetPool:
        default:
            flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
            break;
    }
    VkCommandPoolCreateInfo info{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    info.queueFamilyIndex = queueFamilyIndex;
    info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.flags            = flags;
    info.pNext            = nullptr;
    VK_CHECK_RESULT(vkCreateCommandPool(device.getHandle(), &info, nullptr, &_pool));
}

CommandBuffer CommandPool::allocateCommandBuffer(VkCommandBufferLevel level, bool begin, VkQueueFlags queueFlags) const {
    VkCommandBufferAllocateInfo info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    info.commandBufferCount = 1;
    info.level              = level;
    info.commandPool        = _pool;
    VkCommandBuffer buffer;
    VK_CHECK_RESULT(vkAllocateCommandBuffers(_device.getHandle(), &info, &buffer))

    if (begin) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        VK_CHECK_RESULT(vkBeginCommandBuffer(buffer, &beginInfo));
    }

    return CommandBuffer(buffer, queueFlags);
}
