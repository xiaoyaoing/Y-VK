#pragma once


#include "Core/Vulkan.h"

class Device;

class RenderTarget;

class RenderPass;

class FrameBuffer {
public:
    // FrameBuffer(Device& deivce, std::vector<ImageView>& views, RenderPass& renderPass, VkExtent2D extent);
    FrameBuffer(Device &device, RenderTarget &renderTarget, RenderPass &renderPass, VkExtent2D extent);

    //    explicit FrameBuffer(VkFramebuffer framebuffer) : _framebuffer(framebuffer) {}

    inline VkFramebuffer getHandle() const { return _framebuffer; }

    void cleanup() {
    }

    VkExtent2D getExtent() const;

protected:
    VkFramebuffer _framebuffer;
    Device &device;
    VkExtent2D extent;
};
