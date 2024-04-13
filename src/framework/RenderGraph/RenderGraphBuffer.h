#pragma once
#include "ResourceNode.h"
#include "Common/Enums.h"
#include "Core/Device/Device.h"

class RenderGraphBuffer : public ResourceNode {
public:
    using Usage = BufferUsage;
    struct Descriptor {
        VkDeviceSize   size{};
        Usage          usage;
        VmaMemoryUsage memoryUsage{VMA_MEMORY_USAGE_GPU_ONLY};
    };
    RenderGraphBuffer(const char * name, const Descriptor& descriptor);
    RenderGraphBuffer(const char * name, Buffer* hwBuffer);
    void                       devirtualize() override;
    void                       destroy() override;
    RENDER_GRAPH_RESOURCE_TYPE getType() const override;
    void                       resloveUsage(CommandBuffer& commandBuffer, uint16_t usage) override;
    Buffer*                    getHwBuffer();

private:
    Descriptor mDesc;
    Buffer*    mBuffer{nullptr};
};