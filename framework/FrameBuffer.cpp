#include <RenderTarget.h>
#include <FrameBuffer.h>
#include <RenderPass.h>
#include <Device.h>

FrameBuffer::FrameBuffer(Device &deivce, RenderTarget &renderTarget, RenderPass &renderPass) : device(deivce),
                                                                                               extent(renderTarget.getExtent()) {
    VkFramebufferCreateInfo createInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    auto &views = renderTarget.getViews();
    auto attchments = getHandles<ImageView, VkImageView>(views);
    createInfo.layers = 1;
    createInfo.renderPass = renderPass.getHandle();
    createInfo.attachmentCount = attchments.size();
    createInfo.pAttachments = attchments.data();
    createInfo.height = extent.height;
    createInfo.width = extent.width;

    VK_CHECK_RESULT(vkCreateFramebuffer(deivce.getHandle(), &createInfo, nullptr, &_framebuffer));
}
