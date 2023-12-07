#pragma once

#include <string_view>

#include "RenderGraphId.h"
#include "RenderGraphNode.h"

enum RENDER_GRAPH_RESOURCE_TYPE
{
    UNDEFINED,
    TEXTURE,
    BUFFER,
    COUNT
};


class ResourceNode : public  RenderGraphNode
{
public:
    virtual  void devirtualize() = 0;
    virtual  void destroy() = 0;
    virtual  void resloveUsage(CommandBuffer & commandBuffer) = 0;
    virtual RENDER_GRAPH_RESOURCE_TYPE getType() const = 0;
    virtual std::string getName() const = 0;
public:
    
    PassNode *first{nullptr}, *last{nullptr};
    uint8_t resourceUsage{0};
    RenderGraphHandle handle;
protected:
};
