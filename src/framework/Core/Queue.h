#pragma once

#include "Core/Vulkan.h"

class Device;


class CommandBuffer;

class Queue {
public:
    inline VkQueue getHandle() {
        return _queue;
    }

    ~Queue() {
    }

    Queue(Device *device, int familyIndex, int queueIndex, bool canPresent, const VkQueueFamilyProperties &prop);


    inline bool supportPresent() const { return canPresent; }

    inline uint32_t getFamilyIndex() const { return _familyIndex; }

    inline const VkQueueFamilyProperties &getProp() const { return properties; }


    void submit(const std::vector<VkSubmitInfo> &submit_infos, VkFence fence) const;

    VkResult present(const VkPresentInfoKHR &presentInfo) const;

    void wait();

protected:
    uint32_t _familyIndex, _queueIndex;
    bool canPresent;
    VkQueue _queue;
    Device *_device;
    VkQueueFamilyProperties properties{};
};
