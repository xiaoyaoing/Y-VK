#pragma once


#include "Core/Vulkan.h"

class Device;

class RenderTarget;

class RenderPass;

class FrameBuffer {
public:
    FrameBuffer(Device &device, RenderTarget &renderTarget, RenderPass &renderPass, VkExtent2D extent);
    inline VkFramebuffer getHandle() const { return _framebuffer; }
    VkExtent2D getExtent() const;
protected:
    VkFramebuffer _framebuffer;
    Device &device;
    VkExtent2D extent;
};
