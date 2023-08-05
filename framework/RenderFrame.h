#pragma once

#include <Vulkan.h>
#include "CommandBuffer.h"

class RenderFrame
{
public:
    CommandBuffer &requestCommandBuffer(const Queue &queue, CommandBuffer::ResetMode reset_mode = CommandBuffer::ResetMode::ResetPool,
                                        VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    RenderTarget &getRenderTarget();

protected:
    std::unique_ptr<RenderTarget> render_target;
    Device &device;
};
