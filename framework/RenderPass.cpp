#include "RenderPass.h"
#include <Device.h>
RenderPass::RenderPass(const ptr<Device> &device,
                       const std::vector<VkAttachmentDescription> &attachmentDescs,
                       const std::vector<VkSubpassDependency> &subpassDependencies,
                       const std::vector<VkSubpassDescription> &subpassDesc) {
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = attachmentDescs.size();
    renderPassInfo.pAttachments = attachmentDescs.data();
    renderPassInfo.subpassCount = subpassDesc.size();
    renderPassInfo.pSubpasses = subpassDesc.data();
    renderPassInfo.dependencyCount = subpassDependencies.size();
    renderPassInfo.pDependencies = subpassDependencies.data();
    VK_VERIFY_RESULT(vkCreateRenderPass(device->getHandle(), &renderPassInfo,nullptr,&_pass))
    _device = device;
}

RenderPass::~RenderPass() {
    vkDestroyRenderPass(_device->getHandle(),_pass,nullptr);
    _device.reset();
}
