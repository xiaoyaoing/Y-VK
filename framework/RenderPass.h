#include "Vulkan.h"

class Device;

struct Attachment;

struct SubpassInfo {
    std::vector<uint32_t> inputAttachments;

    std::vector<uint32_t> outputAttachments;

    std::vector<uint32_t> colorResolveAttachments;

    bool disableDepthStencilAttachment;

    uint32_t depthStencilResolveAttachment;

    VkResolveModeFlagBits depthStencilResolveMode;

    std::string debugName;
};


class RenderPass {

public:
    VkRenderPass getHandle() {
        return _pass;
    }

//    RenderPass(const ptr<Device>& device,
//               const std::vector<VkAttachmentDescription>& attachmentDescs,
//               const std::vector<VkSubpassDependency>& subpassDependencies,
//               const std::vector<VkSubpassDescription>& subpassDesc);

    RenderPass(Device &device,
               const std::vector<Attachment> &attachments,
               const std::vector<LoadStoreInfo> &load_store_infos,
               const std::vector<SubpassInfo> &subpasses);

//    RenderPass(const VkRenderPass pass) : _pass(pass) {
//
//    }


    ~RenderPass();

protected:
    VkRenderPass _pass;
    Device &device;

    void setAttachmentLayouts(std::vector<VkSubpassDescription> vector1, const std::vector<Attachment> &vector2);
};