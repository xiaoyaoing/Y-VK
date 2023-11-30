#include "CommandBuffer.h"
#include "Core/Buffer.h"
#include "Core/Images/Image.h"

#include "Core/Descriptor/DescriptorSet.h"
#include "RenderContext.h"
#include "RenderPass.h"
#include "FrameBuffer.h"
#include "Pipeline.h"

#include <algorithm>

void CommandBuffer::beginRecord(VkCommandBufferUsageFlags usage)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = usage; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional
    VK_CHECK_RESULT(vkBeginCommandBuffer(_buffer, &beginInfo))
}


// void CommandBuffer::beginRenderPass(VkRenderPass renderPass, VkFramebuffer buffer,
//                                     const std::vector<VkClearValue>& clearValues,
//                                     const VkExtent2D& extent2D)
// {
//     VkRenderPassBeginInfo renderPassInfo{};
//     renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//     renderPassInfo.renderPass = renderPass;
//     renderPassInfo.framebuffer = buffer;
//     renderPassInfo.renderArea.offset = {0, 0};
//     renderPassInfo.renderArea.extent = extent2D;
//
//     renderPassInfo.clearValueCount = clearValues.size();
//     renderPassInfo.pClearValues = clearValues.data();
//
//
//     vkCmdBeginRenderPass(_buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
// }

void CommandBuffer::bindVertexBuffer(std::vector<const Buffer*>& buffers,
                                     const std::vector<VkDeviceSize>& offsets) const
{
    std::vector<VkBuffer> bufferHandles(buffers.size());
    std::transform(buffers.begin(), buffers.end(), bufferHandles.begin(),
                   [](const Buffer*& buffer) { return buffer->getHandle(); });

    vkCmdBindVertexBuffers(_buffer, 0, static_cast<uint32_t>(bufferHandles.size()), bufferHandles.data(),
                           offsets.data());
}

void CommandBuffer::bindVertexBuffer(uint32_t firstBinding, std::vector<const Buffer*>& buffers,
                                     const std::vector<VkDeviceSize>& offsets) const
{
    std::vector<VkBuffer> bufferHandles(buffers.size());
    std::transform(buffers.begin(), buffers.end(), bufferHandles.begin(),
                   [](const Buffer*& buffer) { return buffer->getHandle(); });

    vkCmdBindVertexBuffers(_buffer, firstBinding, static_cast<uint32_t>(bufferHandles.size()), bufferHandles.data(),
                           offsets.data());
}

void CommandBuffer::bindIndicesBuffer(const Buffer& buffer, VkDeviceSize offset, VkIndexType indexType)
{
    // auto bufferHandles = getHandles<Buffer,VkBuffer>(buffers);
    vkCmdBindIndexBuffer(_buffer, buffer.getHandle(), offset, indexType);
}

void CommandBuffer::bindDescriptorSets(VkPipelineBindPoint bindPoint, VkPipelineLayout layout, uint32_t firstSet,
                                       const std::vector<DescriptorSet>& descriptorSets,
                                       const std::vector<uint32_t>& dynamicOffsets)
{
    std::vector<VkDescriptorSet> vkDescriptorSets(descriptorSets.size(), VK_NULL_HANDLE);
    std::transform(descriptorSets.begin(), descriptorSets.end(), vkDescriptorSets.begin(),
                   [](auto descriptorSet) { return descriptorSet.getHandle(); });
    vkCmdBindDescriptorSets(_buffer, bindPoint, layout, firstSet, vkDescriptorSets.size(), vkDescriptorSets.data(),
                            dynamicOffsets.size(), dynamicOffsets.data());
}

void
CommandBuffer::copyBufferToImage(Buffer& src, Image& dst, const std::vector<VkBufferImageCopy>& copyRegions)
{
    vkCmdCopyBufferToImage(_buffer,
                           src.getHandle(), dst.getHandle(),
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           static_cast<uint32_t>(copyRegions.size()), copyRegions.data());
}

//void CommandBuffer::imageMemoryBarrier(const ImageView& view, ImageMemoryBarrier barrier)
//{
//    auto subresourceRange = view.getSubResourceRange();
//    auto format = view.getFormat();
//    if (isDepthOnlyFormat(format))
//    {
//        subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
//    }
//    else if (isDepthOnlyFormat(format))
//    {
//        subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
//    }
//
//
//    VkImageMemoryBarrier vkBarrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
//    vkBarrier.oldLayout = barrier.oldLayout;
//    vkBarrier.newLayout = barrier.newLayout;
//    vkBarrier.srcAccessMask = barrier.srcAccessMask;
//    vkBarrier.dstAccessMask = barrier.dstAccessMask;
//    vkBarrier.image = view.getImage().getHandle();
//    vkBarrier.srcQueueFamilyIndex = barrier.oldQueueFamily;
//    vkBarrier.dstQueueFamilyIndex = barrier.newQueueFamily;
//    vkBarrier.subresourceRange = subresourceRange;
//
//    vkCmdPipelineBarrier(getHandle(),
//                         barrier.srcStageMask,
//                         barrier.dstStageMask,
//                         0,
//                         0, nullptr,
//                         0, nullptr,
//                         1,
//                         &vkBarrier);
//}

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


// void CommandBuffer::beginRenderPass(RenderPass& render_pass, const std::vector<VkClearValue>& clear_values,
//                                     VkSubpassContents contents)
// {
//     VkRenderPassBeginInfo renderPassInfo{};
//
//     const auto& frameBuffer = RenderContext::g_context->getFrameBuffer();
//
//     renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//     renderPassInfo.renderPass = render_pass.getHandle();
//     renderPassInfo.framebuffer = frameBuffer.getHandle();
//     renderPassInfo.renderArea.offset = {0, 0};
//     renderPassInfo.renderArea.extent = frameBuffer.getExtent();
//
//     renderPassInfo.clearValueCount = clear_values.size();
//     renderPassInfo.pClearValues = clear_values.data();
//
//
//     vkCmdBeginRenderPass(_buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
// }

void CommandBuffer::endRenderPass()
{
    vkCmdEndRenderPass(_buffer);
}

CommandBuffer::~CommandBuffer()
{
}

void CommandBuffer::beginRenderPass(RenderPass& render_pass, FrameBuffer& frameBuffer,
                                    const std::vector<VkClearValue>& clear_values, VkSubpassContents contents)
{
    VkRenderPassBeginInfo renderPassInfo{};


    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = render_pass.getHandle();
    renderPassInfo.framebuffer = frameBuffer.getHandle();
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = frameBuffer.getExtent();

    renderPassInfo.clearValueCount = clear_values.size();
    renderPassInfo.pClearValues = clear_values.data();

    VkClearValue v{.color = {1, 0, 0}};


    vkCmdBeginRenderPass(_buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}


void CommandBuffer::bindPipeline(const Pipeline& pipeline) const
{
    vkCmdBindPipeline(_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getHandle());
}

void CommandBuffer::setViewport(uint32_t firstViewport, const std::vector<VkViewport>& viewports)
{
    vkCmdSetViewport(getHandle(), firstViewport, toUint32(viewports.size()), viewports.data());
}

void CommandBuffer::setScissor(uint32_t firstScissor, const std::vector<VkRect2D>& scissors)
{
    vkCmdSetScissor(getHandle(), firstScissor, toUint32(scissors.size()), scissors.data());
}

void CommandBuffer::bindVertexBuffer(const Buffer& buffer, VkDeviceSize offset)
{
    std::vector<const Buffer*> buffers = {&buffer};
    bindVertexBuffer(buffers, {offset});
}
