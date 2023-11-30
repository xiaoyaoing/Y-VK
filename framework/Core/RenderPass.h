#pragma once

#include "Core/Vulkan.h"

class Device;

struct Attachment;

struct SubpassInfo {
    std::vector<uint32_t> inputAttachments{};

    std::vector<uint32_t> outputAttachments{};

    std::vector<uint32_t> colorResolveAttachments{};

    bool disableDepthStencilAttachment{false};

    uint32_t depthStencilResolveAttachment{};

    VkResolveModeFlagBits depthStencilResolveMode{};

    std::string debugName;
};


class RenderPass {
public:
    RenderPass(RenderPass &&other);

    VkRenderPass getHandle() const {
        return _pass;
    }

    void setHandle(VkRenderPass pass) {
        _pass = pass;
    }

    //    RenderPass(const ptr<Device>& device,
    //               const std::vector<VkAttachmentDescription>& attachmentDescs,
    //               const std::vector<VkSubpassDependency>& subpassDependencies,
    //               const std::vector<VkSubpassDescription>& subpassDesc);

    //    static std::vector<VkClearValue> getClearValues(const std::vector<Attachment>& attachments);

    RenderPass(Device &device, VkRenderPass pass) : _pass(pass), device(device) {
    }

    RenderPass(Device &device,
               const std::vector<Attachment> &attachments,
               const std::vector<SubpassInfo> &subpasses);

    //    RenderPass(const VkRenderPass pass) : _pass(pass) {
    //
    //    }
    const uint32_t getColorOutputCount(uint32_t subpass_index) const;

    ~RenderPass();

protected:
    int temp = 231;
    VkRenderPass _pass;
    Device &device;
    std::vector<uint32_t> colorOutputCount;
};
