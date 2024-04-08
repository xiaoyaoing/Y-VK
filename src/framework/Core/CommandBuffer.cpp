#include "CommandBuffer.h"
#include "Core/Buffer.h"
#include "Core/Images/Image.h"

#include "Core/Descriptor/DescriptorSet.h"
#include "RenderContext.h"
#include "RenderPass.h"
#include "FrameBuffer.h"
#include "Pipeline.h"

#include <algorithm>

void CommandBuffer::beginRecord(VkCommandBufferUsageFlags usage) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags            = usage;  // Optional
    beginInfo.pInheritanceInfo = nullptr;// Optional
    VK_CHECK_RESULT(vkBeginCommandBuffer(mCommandBuffer, &beginInfo))
}

void CommandBuffer::bindVertexBuffer(std::vector<const Buffer*>&      buffers,
                                     const std::vector<VkDeviceSize>& offsets) const {
    std::vector<VkBuffer> bufferHandles(buffers.size());
    std::transform(buffers.begin(), buffers.end(), bufferHandles.begin(), [](const Buffer*& buffer) { return buffer->getHandle(); });

    vkCmdBindVertexBuffers(mCommandBuffer, 0, static_cast<uint32_t>(bufferHandles.size()), bufferHandles.data(), offsets.data());
}

void CommandBuffer::bindVertexBuffer(const Buffer& vertexBuffer) {
    std::vector<VkBuffer>     bufferHandles{vertexBuffer.getHandle()};
    std::vector<VkDeviceSize> offsets{0};
    vkCmdBindVertexBuffers(mCommandBuffer, 0, static_cast<uint32_t>(bufferHandles.size()), bufferHandles.data(), offsets.data());
}

void CommandBuffer::bindVertexBuffer(uint32_t firstBinding, std::vector<const Buffer*>& buffers, const std::vector<VkDeviceSize>& offsets) const {
    std::vector<VkBuffer> bufferHandles(buffers.size());
    std::transform(buffers.begin(), buffers.end(), bufferHandles.begin(), [](const Buffer*& buffer) { return buffer->getHandle(); });

    vkCmdBindVertexBuffers(mCommandBuffer, firstBinding, static_cast<uint32_t>(bufferHandles.size()), bufferHandles.data(), offsets.data());
}

void CommandBuffer::bindIndicesBuffer(const Buffer& buffer, VkDeviceSize offset, VkIndexType indexType) {
    // auto bufferHandles = getHandles<Buffer,VkBuffer>(buffers);
    vkCmdBindIndexBuffer(mCommandBuffer, buffer.getHandle(), offset, indexType);
}

void CommandBuffer::bindDescriptorSets(VkPipelineBindPoint bindPoint, VkPipelineLayout layout, uint32_t firstSet, const std::vector<DescriptorSet>& descriptorSets, const std::vector<uint32_t>& dynamicOffsets) {
    std::vector<VkDescriptorSet> vkDescriptorSets(descriptorSets.size(), VK_NULL_HANDLE);
    std::transform(descriptorSets.begin(), descriptorSets.end(), vkDescriptorSets.begin(), [](const DescriptorSet& descriptorSet) { return descriptorSet.getHandle(); });
    vkCmdBindDescriptorSets(mCommandBuffer, bindPoint, layout, firstSet, vkDescriptorSets.size(), vkDescriptorSets.data(), dynamicOffsets.size(), dynamicOffsets.data());
}

void CommandBuffer::bindDescriptorSets(VkPipelineBindPoint bindPoint, VkPipelineLayout layout, uint32_t firstSet, const std::vector<DescriptorSet*>& descriptorSets, const std::vector<uint32_t>& dynamicOffsets) {
    std::vector<VkDescriptorSet> vkDescriptorSets(descriptorSets.size(), VK_NULL_HANDLE);
    std::transform(descriptorSets.begin(), descriptorSets.end(), vkDescriptorSets.begin(), [](const DescriptorSet* descriptorSet) { return descriptorSet->getHandle(); });
    vkCmdBindDescriptorSets(mCommandBuffer, bindPoint, layout, firstSet, vkDescriptorSets.size(), vkDescriptorSets.data(), dynamicOffsets.size(), dynamicOffsets.data());
}

void CommandBuffer::copyBufferToImage(Buffer& src, Image& dst, const std::vector<VkBufferImageCopy>& copyRegions) {
    vkCmdCopyBufferToImage(mCommandBuffer,
                           src.getHandle(),
                           dst.getHandle(),
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           static_cast<uint32_t>(copyRegions.size()),
                           copyRegions.data());
}

void CommandBuffer::endRenderPass() {
    vkCmdEndRenderPass(mCommandBuffer);
}

CommandBuffer::~CommandBuffer() {
}

void CommandBuffer::beginRenderPass(RenderPass& render_pass, FrameBuffer& frameBuffer, const std::vector<VkClearValue>& clear_values, VkSubpassContents contents) {
    VkRenderPassBeginInfo renderPassInfo{};

    renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass        = render_pass.getHandle();
    renderPassInfo.framebuffer       = frameBuffer.getHandle();
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = frameBuffer.getExtent();

    renderPassInfo.clearValueCount = clear_values.size();
    renderPassInfo.pClearValues    = clear_values.data();

    VkClearValue v{.color = {1, 0, 0}};

    vkCmdBeginRenderPass(mCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBuffer::bindPipeline(const Pipeline& pipeline, VkPipelineBindPoint bindPoint) const {
    vkCmdBindPipeline(mCommandBuffer, bindPoint, pipeline.getHandle());
}

void CommandBuffer::setViewport(uint32_t firstViewport, const std::vector<VkViewport>& viewports) {
    vkCmdSetViewport(getHandle(), firstViewport, toUint32(viewports.size()), viewports.data());
}

void CommandBuffer::setScissor(uint32_t firstScissor, const std::vector<VkRect2D>& scissors) {
    vkCmdSetScissor(getHandle(), firstScissor, toUint32(scissors.size()), scissors.data());
}

void CommandBuffer::bindVertexBuffer(const Buffer& buffer, VkDeviceSize offset) {
    std::vector<const Buffer*> buffers = {&buffer};
    bindVertexBuffer(buffers, {offset});
}
