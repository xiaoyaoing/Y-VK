//
// Created by 打工人 on 2023/3/6.
//

#include "CommandPool.h"
#include "Queue.h"
#include "Device.h"

CommandPool::CommandPool(ptr<Device> device, ptr<Queue> queue, VkCommandPoolCreateFlags flags) : _device(device) {
    VkCommandPoolCreateInfo info{};
    info.queueFamilyIndex = queue->getFamilyIndex();
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.flags = flags;
    info.pNext = nullptr;
    VK_CHECK_RESULT(vkCreateCommandPool(device->getHandle(), &info, nullptr, &_pool));
}

VkCommandBuffer CommandPool::allocateCommandBuffer() {
    VkCommandBuffer cmdBuffer;
    VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
    cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBufferAllocInfo.pNext = nullptr;
    cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBufferAllocInfo.commandBufferCount = 1;
    cmdBufferAllocInfo.commandPool = _pool;
    VK_CHECK_RESULT(vkAllocateCommandBuffers(_device->getHandle(), &cmdBufferAllocInfo, &cmdBuffer));
    return cmdBuffer;
}

CommandPool::CommandPool(Device &device, uint32_t queueFamilyIndex, CommandBuffer::ResetMode resetMode) {

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
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.flags = flags;
    info.pNext = nullptr;
    VK_CHECK_RESULT(vkCreateCommandPool(device.getHandle(), &info, nullptr, &_pool));
}
