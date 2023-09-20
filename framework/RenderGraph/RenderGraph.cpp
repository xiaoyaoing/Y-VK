#include "RenderGraph.h"

void RenderGraph::Builder::read(VirtualResource* resource, PassNode* node)
{
}

void RenderGraph::Builder::write(VirtualResource* resource, PassNode* node)
{
}

void RenderGraph::Builder::declare(const char* name, const RenderGraphPassDescriptor& desc)
{
    auto passNode = static_cast<RenderPassNode*>(node);
}
