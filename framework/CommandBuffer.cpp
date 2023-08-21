#include "CommandBuffer.h"
#include "Buffer.h"
#include "Images/Image.h"
#include "Descriptor/DescriptorSet.h"
#include <RenderPass.h>
#include <FrameBuffer.h>
#include <RenderTarget.h>
#include <algorithm>

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
    std::vector<VkBuffer> bufferHandles(buffers.size());
    std::transform(buffers.begin(), buffers.end(), bufferHandles.begin(),
                   [](ptr<Buffer> &buffer) { return buffer->getHandle(); });

    vkCmdBindVertexBuffers(_buffer, 0, static_cast<uint32_t>(bufferHandles.size()), bufferHandles.data(),
                           offsets.data());
}

void CommandBuffer::bindIndicesBuffer(const ptr<Buffer> &buffer, VkDeviceSize offset) {
    // auto bufferHandles = getHandles<Buffer,VkBuffer>(buffers);
    vkCmdBindIndexBuffer(_buffer, buffer->getHandle(), offset, VK_INDEX_TYPE_UINT16);
}

void CommandBuffer::bindDescriptorSets(VkPipelineBindPoint bindPoint, VkPipelineLayout layout, uint32_t firstSet,
                                       const std::vector<DescriptorSet *> &descriptorSets,
                                       const std::vector<uint32_t> &dynamicOffsets) {
    std::vector<VkDescriptorSet> vkDescriptorSets(descriptorSets.size(), VK_NULL_HANDLE);
    std::transform(descriptorSets.begin(), descriptorSets.end(), vkDescriptorSets.begin(),
                   [](DescriptorSet *descriptorSet) { return descriptorSet->getHandle(); });
    vkCmdBindDescriptorSets(_buffer, bindPoint, layout, firstSet, vkDescriptorSets.size(), vkDescriptorSets.data(),
                            dynamicOffsets.size(), dynamicOffsets.data());
}

void
CommandBuffer::copyBufferToImage(Buffer &src, Image &dst, const std::vector<VkBufferImageCopy> &copyRegions) {
    vkCmdCopyBufferToImage(_buffer,
                           src.getHandle(), dst.getHandle(),
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           static_cast<uint32_t>(copyRegions.size()), copyRegions.data());
}

void CommandBuffer::imageMemoryBarrier(const ImageView &view, ImageMemoryBarrier barrier) {
    VkImageMemoryBarrier vkBarrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    vkBarrier.oldLayout = barrier.oldLayout;
    vkBarrier.newLayout = barrier.newLayout;
    vkBarrier.srcAccessMask = barrier.srcAccessMask;
    vkBarrier.dstAccessMask = barrier.dstAccessMask;
    vkBarrier.image = view.getImage().getHandle();
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

//void CommandBuffer::beginRenderPass(const RenderTarget &render_target, std::unique_ptr<Subpass> &render_pass,
//                                    const FrameBuffer &framebuffer, const std::vector<VkClearValue> &clear_values,
//                                    VkSubpassContents contents) {
////    VkRenderPassBeginInfo renderPassInfo{};
////    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
////    renderPassInfo.renderPass = renderPass;
////    renderPassInfo.framebuffer = buffer;
////    renderPassInfo.renderArea.offset = {0, 0};
////    renderPassInfo.renderArea.extent = extent2D;
////
////    renderPassInfo.clearValueCount = clearValues.size();
////    renderPassInfo.pClearValues = clearValues.data();
////
////
////    vkCmdBeginRenderPass(_buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
//}


void CommandBuffer::beginRenderPass(const RenderTarget &render_target, RenderPass &render_pass,
                                    const FrameBuffer &framebuffer, const std::vector<VkClearValue> &clear_values,
                                    VkSubpassContents contents) {
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = render_pass.getHandle();
    renderPassInfo.framebuffer = framebuffer.getHandle();
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = render_target.getExtent();

    renderPassInfo.clearValueCount = clear_values.size();
    renderPassInfo.pClearValues = clear_values.data();


    vkCmdBeginRenderPass(_buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBuffer::endRenderPass() {
    vkCmdEndRenderPass(_buffer);
}

CommandBuffer::~CommandBuffer() {
}

