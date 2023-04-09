#include "Vulkan.h"
class Device;
class RenderPass {

public:
    VkRenderPass getHandle(){
        return _pass;
    }

    RenderPass(const ptr<Device>& device,
               const std::vector<VkAttachmentDescription>& attachmentDescs,
               const std::vector<VkSubpassDependency>& subpassDependencies,
               const std::vector<VkSubpassDescription>& subpassDesc);

    RenderPass(const VkRenderPass pass) : _pass(pass) {

    }

    ~RenderPass();
protected:
    VkRenderPass _pass;
    ptr<Device> _device;
};