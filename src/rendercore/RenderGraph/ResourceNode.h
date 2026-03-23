#pragma once

#include "Enum.h"

#include <string_view>
#include "Common/BitMaskEnums.h"
#include "RenderGraphId.h"
#include "RenderGraphNode.h"
#include "Common/Enums.h"
#include "Scene/Images/AstcImageHelper.h"

#include <vector>

struct ResourceBarrierInfo {
    std::vector<VkImageMemoryBarrier2KHR> imageBarriers;
    std::vector<VkBufferMemoryBarrier2KHR> bufferBarriers;
    std::vector<VkMemoryBarrier2KHR> memoryBarriers;
};

class ResourceNode : public RenderGraphNode {
public:
    virtual void                       devirtualize()                                             = 0;
    virtual void                       destroy()                                                  = 0;
    virtual void                       resloveUsage(ResourceBarrierInfo & barrierInfo, uint16_t lastUsage, uint16_t nextUsage,RenderPassType lastPassType, RenderPassType nextPassType) = 0;
    virtual uint16_t getDefaultUsage(uint16_t nextUsage) = 0;
    virtual RenderResourceType getType() const                                            = 0;
    
public:
    PassNode *first{nullptr},
        *last{nullptr};
    uint8_t           resourceUsage{0};
    RenderGraphHandle handle;

protected:
};