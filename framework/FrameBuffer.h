#include <Vulkan.h>

class Device;

class RenderTarget;

class RenderPass;

class FrameBuffer {
public:
    FrameBuffer(Device &deivce, RenderTarget &renderTarget, RenderPass &renderPass);

//    explicit FrameBuffer(VkFramebuffer framebuffer) : _framebuffer(framebuffer) {}

    inline VkFramebuffer getHandle() { return _framebuffer; }

    void cleanup() {}

protected:
    VkFramebuffer _framebuffer;
    Device &device;
    VkExtent2D extent;
};