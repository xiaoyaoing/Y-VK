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

std::string RenderGraphBuffer::getName() const
{
    return "buffer";
}

RENDER_GRAPH_RESOURCE_TYPE RenderGraphBuffer::getType() const
{
    return BUFFER;
}

void RenderGraphBuffer::resloveUsage(CommandBuffer& commandBuffer)
{
}
