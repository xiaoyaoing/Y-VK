#pragma once

#include "Vulkan.h"
#include "Images/ImageView.h"
#include "Utils/VkUtils.h"

class Image;

class DescriptorSet;

class Buffer;

class RenderTarget;

class RenderPass;

class FrameBuffer
class CommandBuffer {
public:
    enum class ResetMode {
        ResetPool,
        ResetIndividually,
        AlwaysAllocate,
    };

    inline VkCommandBuffer getHandle() { return _buffer; }

    explicit CommandBuffer(VkCommandBuffer buffer) : _buffer(buffer) {}

    void beginRecord(VkCommandBufferUsageFlags usage);

    void beginRenderPass(VkRenderPass renderPass, VkFramebuffer buffer,
                         const std::vector<VkClearValue> &clearValues, const VkExtent2D &extent2D);

    void beginRenderPass(const RenderTarget &render_target,
                         const RenderPass &render_pass,
                         const FrameBuffer &framebuffer,
                         const std::vector<VkClearValue> &clear_values,
                         VkSubpassContents contents);

    void bindDescriptorSets(VkPipelineBindPoint bindPoint, VkPipelineLayout layout, uint32_t firstSet,
                            const std::vector<ptr<DescriptorSet>> &descriptorSets,
                            const std::vector<uint32_t> &dynamicOffsets);

    void bindVertexBuffer(std::vector<ptr<Buffer>> &buffers, const std::vector<VkDeviceSize> &offsets);

    void bindIndicesBuffer(const ptr<Buffer> &buffer, VkDeviceSize offset);

    void copyBufferToImage(ptr<Buffer>, ptr<Image>, const std::vector<VkBufferImageCopy> &copyRegions);

    //    void bindDescriptor
    inline void bindPipeline(const VkPipeline &pipeline) {
        vkCmdBindPipeline(_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }

    inline void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
        vkCmdDraw(_buffer, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    inline void endRecord() {
        vkEndCommandBuffer(_buffer);
    }

    inline void endPass() {
        vkCmdEndRenderPass(_buffer);
    }

    void imageMemoryBarrier(const ImageView &view, ImageMemoryBarrier barrier);

protected:
    VkCommandBuffer _buffer;
};