#include "CommandBuffer.h"
#include "Buffer.h"
#include "Images/Image.h"
#include "Descriptor/DescriptorSet.h"

void CommandBuffer::beginRecord(VkCommandBufferUsageFlags usage) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = usage; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional
    if (vkBeginCommandBuffer(_buffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command");
    }
}


void CommandBuffer::beginRenderPass(VkRenderPass renderPass, VkFramebuffer buffer,
                                    const std::vector<VkClearValue> &clearValues,
                                    const VkExtent2D &extent2D) {
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = buffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = extent2D;

    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();


    vkCmdBeginRenderPass(_buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBuffer::bindVertexBuffer(std::vector<ptr<Buffer>> &buffers, const std::vector<VkDeviceSize> &offsets) {
    auto bufferHandles = getHandles<Buffer, VkBuffer>(buffers);

    vkCmdBindVertexBuffers(_buffer, 0, static_cast<uint32_t>(bufferHandles.size()), bufferHandles.data(),
                           offsets.data());
}

void CommandBuffer::bindIndicesBuffer(const ptr<Buffer> &buffer, VkDeviceSize offset) {
    // auto bufferHandles = getHandles<Buffer,VkBuffer>(buffers);
    vkCmdBindIndexBuffer(_buffer, buffer->getHandle(), offset, VK_INDEX_TYPE_UINT16);
}

void CommandBuffer::bindDescriptorSets(VkPipelineBindPoint bindPoint, VkPipelineLayout layout, uint32_t firstSet,
                                       const std::vector<ptr<DescriptorSet>> &descriptorSets,
                                       const std::vector<uint32_t> &dynamicOffsets) {
    auto vkDescriptorSets = getHandles<DescriptorSet, VkDescriptorSet>(descriptorSets);
    vkCmdBindDescriptorSets(_buffer, bindPoint, layout, firstSet, vkDescriptorSets.size(), vkDescriptorSets.data(),
                            dynamicOffsets.size(), dynamicOffsets.data());
}

void
CommandBuffer::copyBufferToImage(ptr<Buffer> src, ptr<Image> dst, const std::vector<VkBufferImageCopy> &copyRegions) {
    vkCmdCopyBufferToImage(_buffer,
                           src->getHandle(), dst->getHandle(),
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           static_cast<uint32_t>(copyRegions.size()), copyRegions.data());
}

void CommandBuffer::imageMemoryBarrier(const ImageView &view, ImageMemoryBarrier barrier) {
    VkImageMemoryBarrier vkBarrier{};
    vkBarrier.oldLayout = barrier.oldLayout;
    vkBarrier.newLayout = barrier.newLayout;
    vkBarrier.srcAccessMask = barrier.srcAccessMask;
    vkBarrier.dstAccessMask = barrier.dstAccessMask;
    vkBarrier.image = view.getImage()->getHandle();
    vkBarrier.srcQueueFamilyIndex = barrier.oldQueueFamily;
    vkBarrier.dstQueueFamilyIndex = barrier.newQueueFamily;

    vkCmdPipelineBarrier(getHandle(),
                         barrier.srcStageMask,
                         barrier.dstStageMask,
                         0,
                         0, nullptr,
                         0, nullptr,
                         1,
                         &vkBarrier);
}

void CommandBuffer::beginRenderPass(const RenderTarget &render_target, std::unique_ptr<Subpass> &render_pass,
                                    const FrameBuffer &framebuffer, const std::vector<VkClearValue> &clear_values,
                                    VkSubpassContents contents) {
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = buffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = extent2D;

    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();


    vkCmdBeginRenderPass(_buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}


void CommandBuffer::beginRenderPass(const RenderTarget &render_target, RenderPass &render_pass,
                                    const FrameBuffer &framebuffer, const std::vector<VkClearValue> &clear_values,
                                    VkSubpassContents contents) {

}

void CommandBuffer::endRenderPass() {

}

