//
// Created by 打工人 on 2023/3/6.
//

#include "CommandPool.h"
#include "Engine/Queue.h"
#include "Engine/Device.h"

CommandPool::CommandPool(ptr<Device> device, ptr<Queue> queue, VkCommandPoolCreateFlags flags):_device(device){
    VkCommandPoolCreateInfo info{};
    info.queueFamilyIndex = queue->getFamilyIndex();
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.flags  = flags;
    info.pNext = nullptr;
    ASSERT(vkCreateCommandPool(device->getHandle(),&info, nullptr,&_pool)==VK_SUCCESS,"Create commandPool");
}

VkCommandBuffer CommandPool::allocateCommandBuffer()
{
    VkCommandBuffer cmdBuffer;
    VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
    cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBufferAllocInfo.pNext = nullptr;
    cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBufferAllocInfo.commandBufferCount = 1;
    cmdBufferAllocInfo.commandPool = _pool;
    vkAllocateCommandBuffers(_device->getHandle(), &cmdBufferAllocInfo, &cmdBuffer);
    return cmdBuffer;
}