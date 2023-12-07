#pragma once
#include "ResourceNode.h"
#include "Common/Enums.h"
#include "Core/Device/Device.h"

class RenderGraphBuffer : public  ResourceNode
{
public:
    using Usage = BufferUsage;
    class Descriptor
    {
        VkDeviceSize size{};
        VkBufferUsageFlagBits usage{};
        VmaMemoryUsage memoryUsage{};
    };
    RenderGraphBuffer(const std::string& name, const Descriptor& descriptor);
    RenderGraphBuffer(const std::string& name, const Buffer *  hwBuffer);
    void devirtualize() override;
    void destroy() override;
    std::string getName() const override;
    RENDER_GRAPH_RESOURCE_TYPE getType() const override;
    void resloveUsage(CommandBuffer& commandBuffer) override;

private:
    
};
