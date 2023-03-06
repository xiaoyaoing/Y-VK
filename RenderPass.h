#include "Vulkan.h"
class RenderPass {

public:
    VkRenderPass getHandle(){
        return _pass;
    }

    RenderPass(const VkRenderPass pass) : _pass(pass) {

    }

    void cleanup(){

    }

protected:
    VkRenderPass _pass;
};