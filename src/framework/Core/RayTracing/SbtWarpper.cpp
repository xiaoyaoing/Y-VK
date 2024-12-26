#include "SbtWarpper.h"

#include <vectypes.h>

#include "Core/Device/Device.h"

SbtWarpper::SbtWarpper(Device& device):device(device)
{
}

uint32_t SbtWarpper::indexCount(GroupType t)
{
    return mIndex[t].size();
}

VkDeviceAddress SbtWarpper::getAddress(GroupType t)
{
    if (!mBuffer[t]) {
        return 0;
    }
    VkBufferDeviceAddressInfo i{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, mBuffer[t]->getHandle()};
    return vkGetBufferDeviceAddress(device.getHandle(), &i);
}

void SbtWarpper::addIndices(VkRayTracingPipelineCreateInfoKHR info)
{
    uint32_t groupIndex = 0;
    uint32_t stageIdx = 0;
    uint32_t group_offset = 0;
    for(uint32_t  g = 0 ;g< info.groupCount;g++)
    {
        if (info.pGroups[g].type == VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR) {
            if (info.pStages[stageIdx].stage == VK_SHADER_STAGE_RAYGEN_BIT_KHR) {
                mIndex[eRaygen].push_back(g + group_offset);
                stageIdx++;
            } else if (info.pStages[stageIdx].stage == VK_SHADER_STAGE_MISS_BIT_KHR) {
                mIndex[eMiss].push_back(g + group_offset);
                stageIdx++;
            } else if (info.pStages[stageIdx].stage == VK_SHADER_STAGE_CALLABLE_BIT_KHR) {
                mIndex[eCallable].push_back(g + group_offset);
                stageIdx++;
            }
        } else {
            mIndex[eHit].push_back(g + group_offset);
            if (info.pGroups[g].closestHitShader != VK_SHADER_UNUSED_KHR) stageIdx++;
            if (info.pGroups[g].anyHitShader != VK_SHADER_UNUSED_KHR) stageIdx++;
            if (info.pGroups[g].intersectionShader != VK_SHADER_UNUSED_KHR) stageIdx++;
        }
    }
}

void SbtWarpper::create(VkPipeline rtPipeline, VkRayTracingPipelineCreateInfoKHR info)
{
    auto & rtProp = device.getVkPhysicalDeviceRayTracingPipelineProperties();
    const uint32_t handle_size         = rtProp.shaderGroupHandleSize;
    const uint32_t handle_alignment    = rtProp.shaderGroupHandleAlignment;
    
    addIndices(info);
    uint32_t total_group_cnt = info.groupCount;

    uint32_t sbtSize = total_group_cnt * handle_size;
    std::vector<uint8_t> shader_handle_storage(sbtSize);
    
    VK_CHECK_RESULT(vkGetRayTracingShaderGroupHandlesKHR(device.getHandle(), rtPipeline, 0, total_group_cnt, sbtSize,
                                                       shader_handle_storage.data()));


    
    
    const uint32_t handle_size_aligned = alignUp(handle_size, handle_alignment);

    std::ranges::fill(mStride.begin(), mStride.end(), handle_size_aligned);

    auto copyHandles = [&](auto &buffer, std::vector<uint32_t> &indices, uint32_t stride) {
        auto *pBuffer = buffer.data();
        for (uint32_t index = 0; index < static_cast<uint32_t>(indices.size()); index++)
        {
            auto *pStart = pBuffer;
            memcpy(pBuffer, shader_handle_storage.data() + (indices[index] * handle_size), handle_size);
            pBuffer = pStart + stride;       
        }
    };

    std::array<std::vector<uint8_t>, 4> stage;
    stage[eRaygen] = std::vector<uint8_t>(mStride[eRaygen] * indexCount(eRaygen));
    stage[eMiss] = std::vector<uint8_t>(mStride[eMiss] * indexCount(eMiss));
    stage[eHit] = std::vector<uint8_t>(mStride[eHit] * indexCount(eHit));
    stage[eCallable] = std::vector<uint8_t>(mStride[eCallable] * indexCount(eCallable));

    

    copyHandles(stage[eRaygen], mIndex[eRaygen], mStride[eRaygen]);
    copyHandles(stage[eMiss], mIndex[eMiss], mStride[eMiss]);
    copyHandles(stage[eHit], mIndex[eHit], mStride[eHit]);
    copyHandles(stage[eCallable], mIndex[eCallable], mStride[eCallable]);


    const VkBufferUsageFlags sbt_buffer_usage_flags = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    const VmaMemoryUsage     sbt_memory_usage       = VMA_MEMORY_USAGE_CPU_TO_GPU;
    for(int i = 0;i<4;i++)
    {
        if(!stage[i].empty())
        mBuffer[i] = std::make_unique<Buffer>(device, stage[i].size(),
            sbt_buffer_usage_flags,sbt_memory_usage,stage[i].data());
    }
}

const VkStridedDeviceAddressRegionKHR SbtWarpper::getRegion(GroupType t)
{
    if(!mBuffer[t])
        return {};
    return VkStridedDeviceAddressRegionKHR{getAddress(t), mStride[t], indexCount(t) * mStride[t]};
}

const std::array<VkStridedDeviceAddressRegionKHR, 4> SbtWarpper::getRegions()
{
    return std::array<VkStridedDeviceAddressRegionKHR, 4>{getRegion(eRaygen), getRegion(eMiss), getRegion(eHit), getRegion(eCallable)};
}
