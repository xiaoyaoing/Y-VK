#pragma once
#include <Vulkan.h>
class Device;
class Fence;
class CommandBuffer;
class Queue {
public:
    inline VkQueue getHandle() {
        return _queue;
    }

    Queue(const ptr<Device> & device,int familyIndex);
    void submit(const std::vector<ptr<CommandBuffer>>& cmdBuffers,
                ptr<Fence> fence);

    inline  uint32_t getFamilyIndex(){return _familyIndex;}
protected:
    uint32_t _familyIndex;
    VkQueue  _queue;
    ptr<Device> _device;
};
