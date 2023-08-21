#pragma once

#include "Vulkan.h"
#include "Images/ImageView.h"
#include "Utils/VkUtils.h"

class Image;

class DescriptorSet;

class Buffer;

class RenderTarget;

class RenderPass;

class Subpass;

class FrameBuffer;

class CommandBuffer {
public:
    enum class ResetMode {
        ResetPool,
        ResetIndividually,
        AlwaysAllocate,
    };

    VkCommandBuffer getHandle() const { return _buffer; }

    explicit CommandBuffer(VkCommandBuffer buffer) : _buffer(buffer) {}

    void beginRecord(VkCommandBufferUsageFlags usage);

    void beginRenderPass(VkRenderPass renderPass, VkFramebuffer buffer,
                         const std::vector<VkClearValue> &clearValues, const VkExtent2D &extent2D);

//    void beginRenderPass(const RenderTarget &render_target,
//                         std::unique_ptr<Subpass> &render_pass,
//                         const FrameBuffer &framebuffer,
//                         const std::vector<VkClearValue> &clear_values,
//                         VkSubpassContents contents);

    void beginRenderPass(const RenderTarget &render_target,
                         RenderPass &render_pass,
                         const FrameBuffer &framebuffer,
                         const std::vector<VkClearValue> &clear_values,
                         VkSubpassContents contents);

    void bindDescriptorSets(VkPipelineBindPoint bindPoint, VkPipelineLayout layout, uint32_t firstSet,
                            const std::vector<DescriptorSet *> &descriptorSets,
                            const std::vector<uint32_t> &dynamicOffsets);

    void bindVertexBuffer(std::vector<ptr<Buffer>> &buffers, const std::vector<VkDeviceSize> &offsets);

    void bindIndicesBuffer(const ptr<Buffer> &buffer, VkDeviceSize offset);

    void copyBufferToImage(Buffer &, Image &, const std::vector<VkBufferImageCopy> &copyRegions);

    //    void bindDescriptor
    inline void bindPipeline(const VkPipeline &pipeline) {
        vkCmdBindPipeline(_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }

    inline void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
        vkCmdDraw(_buffer, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    inline void drawIndexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset,
                            uint32_t first_instance) {

        vkCmdDrawIndexed(_buffer, index_count, instance_count, first_index, vertex_offset, first_instance);
    }

    inline void endRecord() {
        VK_CHECK_RESULT(vkEndCommandBuffer(_buffer));
    }

    inline void endPass() {
        vkCmdEndRenderPass(_buffer);
    }

    void imageMemoryBarrier(const ImageView &view, ImageMemoryBarrier barrier);

    void endRenderPass();

    ~CommandBuffer();

protected:
    VkCommandBuffer _buffer;
};