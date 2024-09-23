//
// Created by 打工人 on 2023/3/6.
//
#include "Core/Device/Device.h"
#include "Queue.h"
#include "Core/CommandBuffer.h"

Queue::Queue(Device* device, int familyIndex, int queueIndex, bool canPresent, const VkQueueFamilyProperties& prop)
    : _familyIndex(familyIndex),
      _queueIndex(queueIndex),
      canPresent(canPresent),
      _device(device),
      properties(prop)
{
    vkGetDeviceQueue(device->getHandle(), familyIndex, _queueIndex, &_queue);
}


void Queue::submit(const std::vector<VkSubmitInfo>& submit_infos, VkFence fence) const
{

    auto result = vkQueueSubmit(_queue, uint32_t(submit_infos.size()), submit_infos.data(), fence);
    if (result != VK_SUCCESS)
    {
        LOGI("Failed to submit command buffer to queue.");
    }
}


VkResult Queue::present(const VkPresentInfoKHR& presentInfo) const
{
    return vkQueuePresentKHR(_queue, &presentInfo);
}

void Queue::wait()
{
    vkQueueWaitIdle(_queue);
}
