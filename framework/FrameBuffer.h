#include "Vulkan.h"

class Device;

class FrameBuffer {
public:
    explicit FrameBuffer(VkFramebuffer framebuffer) : _framebuffer(framebuffer) {}

    inline VkFramebuffer getHandle() { return _framebuffer; }

    void cleanup() {}

protected:
    VkFramebuffer _framebuffer;
};