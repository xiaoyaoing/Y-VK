//
// Created by 打工人 on 2023/3/6.
//
#include "Device.h"
#include "Queue.h"
#include "CommandBuffer.h"
#include <Sync/Fence.h>

Queue::Queue(Device *device, int familyIndex, int queueIndex, bool canPresent, const VkQueueFamilyProperties &prop)
        : _device(device),
          _familyIndex(familyIndex),
          _queueIndex(queueIndex),
          canPresent(canPresent),
          properties(prop) {
    vkGetDeviceQueue(device->getHandle(), familyIndex, _queueIndex, &_queue);
}

void Queue::submit(const std::vector<ptr<CommandBuffer>> &cmdBuffers, ptr<Fence> fence) {
//    auto vkCmdBuffers = getHandles<CommandBuffer, VkCommandBuffer>(cmdBuffers);
    std::vector<VkCommandBuffer> vkCmdBuffers(cmdBuffers.size());
    std::transform(cmdBuffers.begin(), cmdBuffers.end(), vkCmdBuffers.begin(),
                   [](const ptr<CommandBuffer> &buffer) -> VkCommandBuffer {
                       return buffer->getHandle();
                   });
    VkSubmitInfo submitInfo{};
    submitInfo.pCommandBuffers = vkCmdBuffers.data();
    submitInfo.commandBufferCount = vkCmdBuffers.size();
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    vkQueueSubmit(_queue, 1, &submitInfo, fence == nullptr ? VK_NULL_HANDLE : fence->getHandle());
}

VkResult Queue::submit(const std::vector<VkSubmitInfo> &submit_infos, VkFence fence) const {

    return vkQueueSubmit(_queue, uint32_t(submit_infos.size()), submit_infos.data(), fence);

}


VkResult Queue::present(const VkPresentInfoKHR &presentInfo) const {
    return vkQueuePresentKHR(_queue, &presentInfo);
}


