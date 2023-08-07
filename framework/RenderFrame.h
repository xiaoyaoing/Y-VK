#pragma once

#include <Vulkan.h>
#include "CommandBuffer.h"

class Queue;

class RenderFrame {
public:
    CommandBuffer &
    requestCommandBuffer(const Queue &queue, CommandBuffer::ResetMode reset_mode = CommandBuffer::ResetMode::ResetPool,
                         VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    RenderTarget &getRenderTarget();

    RenderFrame(Device &device, std::unique_ptr<RenderTarget> &&renderTarget);

    void reset();

protected:
    std::unique_ptr<RenderTarget> renderTarget;
    Device &device;
};
