#pragma once
#include <array>

#include "Core/Buffer.h"

class SbtWarpper
{
public:
    enum GroupType{ eRaygen, eMiss, eHit, eCallable };
    SbtWarpper(Device & device);
    VkDeviceAddress getAddress(GroupType t);
    uint32_t indexCount(GroupType t);
    void addIndices(VkRayTracingPipelineCreateInfoKHR pipelineInfo);
    void create(VkPipeline rtPipeline, VkRayTracingPipelineCreateInfoKHR pipelineInfo);

    uint32_t getStride(GroupType t) { return mStride[t]; }
    uint32_t getSize(GroupType t) { return getStride(t) * indexCount(t); }
    const VkStridedDeviceAddressRegionKHR getRegion(GroupType t);
    const std::array<VkStridedDeviceAddressRegionKHR, 4> getRegions();
protected:
    Device & device;
    std::array<std::unique_ptr<Buffer>,4> mBuffer;
    std::array<std::vector<uint32_t>,4> mIndex;
    std::array<uint32_t, 4> mStride{0, 0, 0, 0};
    // uint32_t m_handle_size{0};
    // uint32_t m_handle_alignment{0};
};

