#pragma once
#include "Enum.h"
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
    RenderGraphBuffer(const std::string& name, const Descriptor& descriptor);
    RenderGraphBuffer(const std::string& name, Buffer* hwBuffer);
    void                       devirtualize() override;
    void                       destroy() override;
    RenderResourceType         getType() const override;
    void                       resloveUsage(ResourceBarrierInfo& barrierInfo, uint16_t lastUsage, uint16_t nextUsage, RenderPassType lastPassType, RenderPassType nextPassType) override;
    Buffer*                    getHwBuffer();

private:
    Descriptor mDesc;
    Buffer*    mBuffer{nullptr};
};