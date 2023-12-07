#include "RenderGraphBuffer.h"

RenderGraphBuffer::RenderGraphBuffer(const std::string& name, const Descriptor& descriptor)
{
}

RenderGraphBuffer::RenderGraphBuffer(const std::string& name, const Buffer* hwBuffer)
{
}

void RenderGraphBuffer::devirtualize()
{
}

void RenderGraphBuffer::destroy()
{
}

RENDER_GRAPH_RESOURCE_TYPE RenderGraphBuffer::getType() const
{
    return BUFFER;
}

void RenderGraphBuffer::resloveUsage(CommandBuffer& commandBuffer)
{
}
