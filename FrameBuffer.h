#include "Vulkan.h"

class Device;
class Framebuffer {
public:
    explicit Framebuffer(VkFramebuffer framebuffer) : _framebuffer(framebuffer){}
    inline  VkFramebuffer  getHandle(){return _framebuffer;}
    void cleanup(){}
protected:
    VkFramebuffer  _framebuffer;
};