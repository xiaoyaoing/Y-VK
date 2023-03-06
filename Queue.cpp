//
// Created by 打工人 on 2023/3/6.
//
#include <Device.h>
#include "Queue.h"
#include <CommandBuffer.h>
#include <Sync/Fence.h>

Queue::Queue(const ptr<Device> &device, int familyIndex) : _device(device),_familyIndex(familyIndex){
    vkGetDeviceQueue(device->getHandle(),familyIndex,0,&_queue);
}

void Queue::submit(const std::vector<ptr<CommandBuffer>> &cmdBuffers, ptr<Fence> fence) {
    auto vkCmdBuffers = getHandles<CommandBuffer,VkCommandBuffer>(cmdBuffers);
    VkSubmitInfo submitInfo{};
    submitInfo.pCommandBuffers  = vkCmdBuffers.data();
    submitInfo.commandBufferCount = vkCmdBuffers.size();
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    vkQueueSubmit(_queue,1,&submitInfo,fence == nullptr?VK_NULL_HANDLE:fence->getHandle());
}

