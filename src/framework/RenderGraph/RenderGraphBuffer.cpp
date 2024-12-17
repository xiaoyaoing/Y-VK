#include "RenderGraphBuffer.h"
#include "Common/ResourceCache.h"
#include "Core/Images/ImageUtil.h"

RenderGraphBuffer::RenderGraphBuffer(const std::string& name, const Descriptor& descriptor) {
    //todo
    //mBuffer = ResourceCache::getResourceCache().requestNamedBuffer(name, descriptor.size, descriptor.usage, descriptor.memoryUsage);
}

RenderGraphBuffer::RenderGraphBuffer(const std::string& name, Buffer* hwBuffer) {
    RenderGraphNode::setName(name);

    mBuffer = hwBuffer;
}

void RenderGraphBuffer::devirtualize() {
}

void RenderGraphBuffer::destroy() {
    delete this;
    //todo
}

RenderResourceType RenderGraphBuffer::getType() const {
    return RenderResourceType::EBuffer;
}

VkAccessFlags2 getAccessFlags(BufferUsage usage) {
    VkAccessFlags2 flags = VK_ACCESS_2_NONE;
    if(any(usage & BufferUsage::TRANSFER_SRC))
        flags |= VK_ACCESS_TRANSFER_READ_BIT;
    if(any(usage & BufferUsage::TRANSFER_DST))
        flags |= VK_ACCESS_TRANSFER_WRITE_BIT;
    if(any(usage & BufferUsage::STORAGE))
        flags |= VK_ACCESS_SHADER_WRITE_BIT;
    if(any(usage & BufferUsage::UPLOADABLE))
        flags |= VK_ACCESS_HOST_WRITE_BIT;
    if(any(usage & BufferUsage::RAY_TRACING))
        flags |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    if(any(usage & BufferUsage::INDIRECT))
        flags |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
    if(any(usage & BufferUsage::READ))
        flags |= VK_ACCESS_SHADER_READ_BIT;
    return flags;
}

VkPipelineStageFlags2 getDefaultPipelineStageFlags(BufferUsage usage) {
    VkPipelineStageFlags2 flags = VK_PIPELINE_STAGE_2_NONE;
    
    if(any(usage & BufferUsage::TRANSFER_SRC) || any(usage & BufferUsage::TRANSFER_DST))
        flags |= VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        
    if(any(usage & BufferUsage::STORAGE))
        flags |= VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        
    if(any(usage & BufferUsage::UPLOADABLE))
        flags |= VK_PIPELINE_STAGE_2_HOST_BIT;
        
    if(any(usage & BufferUsage::RAY_TRACING))
        flags |= VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR;
        
    if(any(usage & BufferUsage::INDIRECT))
        flags |= VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        
    if(any(usage & BufferUsage::READ))
        flags |= VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;

    if(flags == VK_PIPELINE_STAGE_2_NONE)
        flags = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        
    return flags;
}

void RenderGraphBuffer::resloveUsage(ResourceBarrierInfo& barrierInfo, uint16_t lastUsage, uint16_t nextUsage, RenderPassType lastPassType, RenderPassType nextPassType) {
    auto & bufferBarrier = barrierInfo.bufferBarriers.emplace_back();
    bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
    if (lastPassType!=RenderPassType::UNDEFINED)
        bufferBarrier.srcStageMask = ImageUtil::getStageFlags(lastPassType);
    else {
        bufferBarrier.srcStageMask = getDefaultPipelineStageFlags(static_cast<BufferUsage>(lastUsage));
    }
    bufferBarrier.dstStageMask = ImageUtil::getStageFlags(nextPassType);
    bufferBarrier.srcAccessMask = getAccessFlags(static_cast<BufferUsage>(lastUsage));
    bufferBarrier.dstAccessMask = getAccessFlags(static_cast<BufferUsage>(nextUsage));
    bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bufferBarrier.buffer = mBuffer->getHandle();
    bufferBarrier.size = mBuffer->getSize();
}


Buffer* RenderGraphBuffer::getHwBuffer() {
    return mBuffer;
}

uint16_t RenderGraphBuffer::getDefaultUsage(uint16_t nextUsage) {
    return nextUsage;
}