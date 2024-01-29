#pragma once

#include <string_view>
#include "Common/BitMaskEnums.h"
#include "RenderGraphId.h"
#include "RenderGraphNode.h"

enum class RENDER_GRAPH_RESOURCE_TYPE : uint8_t {
    UNDEFINED = 0,
    ETexture  = 1,
    EBuffer   = 1 << 1,
    ALL       = ETexture | EBuffer,
};

template<>
struct EnableBitMaskOperators<RENDER_GRAPH_RESOURCE_TYPE> : std::true_type {};

class ResourceNode : public RenderGraphNode {
public:
    virtual void                       devirtualize()                                             = 0;
    virtual void                       destroy()                                                  = 0;
    virtual void                       resloveUsage(CommandBuffer& commandBuffer, uint16_t usage) = 0;
    virtual RENDER_GRAPH_RESOURCE_TYPE getType() const                                            = 0;

public:
    PassNode *first{nullptr},
        *last{nullptr};
    uint8_t           resourceUsage{0};
    RenderGraphHandle handle;

protected:
};