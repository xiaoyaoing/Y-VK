#pragma once

#include <unordered_map>

#include "Shader.h"
#include "RenderPass.h"

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/hash.hpp>

#include <mutex>

#include "FrameBuffer.h"
#include "Pipeline.h"
#include "RenderTarget.h"
#include "Descriptor/DescriptorLayout.h"
#include "Descriptor/DescriptorPool.h"
#include "Descriptor/DescriptorSet.h"


//A resource cache  system
//Mainly from vulkan-samples


struct ResourceCacheState
{
    std::unordered_map<std::size_t, Shader> shader_modules;

    std::unordered_map<std::size_t, PipelineLayout> pipeline_layouts;

    std::unordered_map<std::size_t, Pipeline> pipelines;

    std::unordered_map<std::size_t, DescriptorLayout> descriptor_set_layouts;

    std::unordered_map<std::size_t, DescriptorPool> descriptor_pools;

    std::unordered_map<std::size_t, DescriptorSet> descriptorSets;

    std::unordered_map<std::size_t, RenderPass> render_passes;

    std::unordered_map<std::size_t, sg::SgImage> sgImages;

    std::unordered_map<std::size_t, FrameBuffer> frameBuffers;
};


template <class T, class... A>
T& requestResource(Device& device, std::unordered_map<std::size_t, T>& resources, A&... args)
{
    std::size_t hash{0U};
    hash_param(hash, args...);
    auto res = resources.find(hash);
    if (res != resources.end())
    {
        return res->second;
    }

    T resource(device, args...);
    auto res_it = resources.emplace(hash, std::move(resource));

    res = res_it.first;
    return res->second;

    // return resources[hash]->second;
}

class ResourceCache
{
public:
    // std::unordered_map<Rende>

    static ResourceCache& getResourceCache();

    static void initCache(Device& device);

    sg::SgImage& requestSgImage(const std::string& path, VkImageViewType viewType);

    sg::SgImage& requestSgImage(const std::string& name,
                                const VkExtent3D& extent,
                                VkFormat format,
                                VkImageUsageFlags image_usage,
                                VmaMemoryUsage memory_usage,
                                VkImageViewType viewType,
                                VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT,
                                uint32_t mip_levels = 1,
                                uint32_t array_layers = 1,
                                VkImageCreateFlags flags = 0);


    RenderPass& requestRenderPass(const std::vector<Attachment>& attachments,
                                  const std::vector<SubpassInfo>& subpasses);

    DescriptorLayout& requestDescriptorLayout(std::vector<Shader>& shaders);

    PipelineLayout& requestPipelineLayout(std::vector<Shader>& shaders);

    FrameBuffer& requestFrameBuffer(RenderTarget& renderTarget, RenderPass& renderPass);

    DescriptorSet& requestDescriptorSet(const DescriptorLayout& descriptorSetLayout,
                                        DescriptorPool& descriptorPool,
                                        const BindingMap<VkDescriptorBufferInfo>& bufferInfos,
                                        const BindingMap<VkDescriptorImageInfo>& imageInfos);

    DescriptorPool& requestDescriptorPool(const DescriptorLayout& layout,
                                          uint32_t poolSize = DescriptorPool::MAX_SETS_PER_POOL);


    ResourceCache(Device& device);

    Pipeline& requestPipeline(const PipelineState& state);

private:
    static std::unique_ptr<ResourceCache> cache;

    ResourceCacheState state;

    Device& device;

    std::mutex renderPassMutex;

    std::mutex descriptorLayoutMutex;

    std::mutex descriptorPoolMutex;

    std::mutex descriptorSetMutex;

    std::mutex frameBufferMutex;

    std::mutex pipelineMutex;

    std::mutex pipelineLayoutMutex;

    std::mutex sgImageMutex;

    VkPipelineCache pipelineCache;
};
