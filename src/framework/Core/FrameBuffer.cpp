#include "RenderTarget.h"
#include "FrameBuffer.h"
#include "RenderPass.h"
#include "Core/Device/Device.h"


VkExtent2D FrameBuffer::getExtent() const {
    return extent;
}

FrameBuffer::FrameBuffer(Device &device, RenderTarget &renderTarget, RenderPass &renderPass) : device(device),
                                                                                               extent(renderTarget.getExtent()) {

    
    VkFramebufferCreateInfo createInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    auto hwTextures = renderTarget.getHwTextures();
    std::vector<VkImageView> attachments(hwTextures.size(), VK_NULL_HANDLE);
    std::transform(hwTextures.begin(), hwTextures.end(), attachments.begin(),
                   [](auto &hwTexture) -> VkImageView {
                       return hwTexture->getVkImageView().getHandle();
                   });
    createInfo.layers = 1;
    createInfo.renderPass = renderPass.getHandle();
    createInfo.attachmentCount = toUint32(attachments.size());
    createInfo.pAttachments = attachments.data();
    createInfo.height = extent.height;
    createInfo.width = extent.width;
    VK_CHECK_RESULT(vkCreateFramebuffer(device.getHandle(), &createInfo, nullptr, &_framebuffer))
}
