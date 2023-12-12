#pragma once

#include "FrameBuffer.h"
#include "Vulkan.h"
#include "Common/VkUtils.h"
#include "PipelineState.h"

class Image;

class ImageView;

class DescriptorSet;

class Buffer;

class RenderTarget;

class RenderPass;

class Subpass;

class FrameBuffer;

class Pipeline;

class CommandBuffer {
public:
    enum class ResetMode {
        ResetPool,
        ResetIndividually,
        AlwaysAllocate,
    };


    VkCommandBuffer getHandle() const { return mCommandBuffer; }

    //Avoid right value can't be located.
    const VkCommandBuffer *getHandlePointer() const { return &mCommandBuffer; }


    explicit CommandBuffer(VkCommandBuffer buffer) : mCommandBuffer(buffer) {
    }

    void beginRecord(VkCommandBufferUsageFlags usage);


    void beginRenderPass(RenderPass &render_pass, FrameBuffer &frameBuffer,
                         const std::vector<VkClearValue> &clear_values,
                         VkSubpassContents contents);

    void bindDescriptorSets(VkPipelineBindPoint bindPoint, VkPipelineLayout layout, uint32_t firstSet,
                            const std::vector<DescriptorSet> &descriptorSets,
                            const std::vector<uint32_t> &dynamicOffsets);

    void bindVertexBuffer(std::vector<const Buffer *> &buffers, const std::vector<VkDeviceSize> &offsets) const;

    void bindVertexBuffer(uint32_t firstBinding, std::vector<const Buffer *> &buffers,
                          const std::vector<VkDeviceSize> &offsets) const;

    void bindVertexBuffer( const Buffer & buffer);

    void bindVertexBuffer(const Buffer &buffers, VkDeviceSize offset);


    void bindIndicesBuffer(const Buffer &buffer, VkDeviceSize offset, VkIndexType indexType = VK_INDEX_TYPE_UINT16);

    void copyBufferToImage(Buffer &, Image &, const std::vector<VkBufferImageCopy> &copyRegions);

    inline void bindPipeline(const VkPipeline &pipeline) const {
        vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }

    void bindPipeline(const Pipeline &pipeline,VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS) const;


    inline void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
        vkCmdDraw(mCommandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    inline void drawIndexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset,
                            uint32_t first_instance) {
        vkCmdDrawIndexed(mCommandBuffer, index_count, instance_count, first_index, vertex_offset, first_instance);
    }


    void setViewport(uint32_t firstViewport, const std::vector<VkViewport> &viewports);

    void setScissor(uint32_t firstScissor, const std::vector<VkRect2D> &scissors);

    inline void endRecord() {
        VK_CHECK_RESULT(vkEndCommandBuffer(mCommandBuffer))
    }

    inline void endPass() {
        vkCmdEndRenderPass(mCommandBuffer);
    }

    void endRenderPass();

    

    ~CommandBuffer();

protected:
    VkCommandBuffer mCommandBuffer;

};
