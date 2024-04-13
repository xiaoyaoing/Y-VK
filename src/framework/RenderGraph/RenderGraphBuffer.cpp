#include "RenderGraphBuffer.h"
#include "Common/ResourceCache.h"

RenderGraphBuffer::RenderGraphBuffer(const char * name, const Descriptor& descriptor) {
    //todo
    //mBuffer = ResourceCache::getResourceCache().requestNamedBuffer(name, descriptor.size, descriptor.usage, descriptor.memoryUsage);
}

RenderGraphBuffer::RenderGraphBuffer(const char * name, Buffer* hwBuffer){
    RenderGraphNode::setName(name);

    mBuffer = hwBuffer;
}

void RenderGraphBuffer::devirtualize() {
}

void RenderGraphBuffer::destroy() {
    delete this;
    //todo
}

RENDER_GRAPH_RESOURCE_TYPE RenderGraphBuffer::getType() const {
    return RENDER_GRAPH_RESOURCE_TYPE::EBuffer;
}

void RenderGraphBuffer::resloveUsage(CommandBuffer& commandBuffer, uint16_t usage) {
}

Buffer* RenderGraphBuffer::getHwBuffer() {
    return mBuffer;
}