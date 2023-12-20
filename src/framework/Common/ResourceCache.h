#pragma once

#include <unordered_map>

#include "Core/Shader/Shader.h"
#include "Core/RenderPass.h"

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/hash.hpp>

#include <mutex>

#include "Core/FrameBuffer.h"
#include "Core/Pipeline.h"
#include "Core/RenderTarget.h"
#include "Core/ResourceCachingHelper.h"
#include "Core/Descriptor/DescriptorLayout.h"
#include "Core/Descriptor/DescriptorPool.h"
#include "Core/Descriptor/DescriptorSet.h"


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

    std::unordered_map<std::size_t, SgImage> sgImages;

    std::unordered_map<std::size_t, FrameBuffer> frameBuffers;
    
    std::unordered_map<std::size_t, Buffer> buffers;
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


    LOGI("Creating resource of type %s", typeid(T).name());
    
    T resource(device, args...);
    auto res_it = resources.emplace(hash, std::move(resource));

    res = res_it.first;
    return res->second;

    // return resources[hash]->second;
}


template <class T, class... A>
T& requestResource(Device& device,const std::string & name, std::unordered_map<std::size_t, T>& resources, A&... args)
{
    std::size_t hash{0U};
    hash_param(hash, name);
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
    ~ResourceCache() = default;

    static ResourceCache& getResourceCache();

    static void initCache(Device& device);

    SgImage& requestSgImage(const std::string& path, VkImageViewType viewType);

    SgImage& requestSgImage(const std::string& name,
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
                                        const BindingMap<VkDescriptorImageInfo>& imageInfos,
                                        const BindingMap<VkWriteDescriptorSetAccelerationStructureKHR> & accelerations);

    DescriptorPool& requestDescriptorPool(const DescriptorLayout& layout,
                                          uint32_t poolSize = DescriptorPool::MAX_SETS_PER_POOL);

    Buffer & requestNamedBuffer(const std::string & name, uint64_t bufferSize, VkBufferUsageFlags bufferUsage,
                    VmaMemoryUsage memoryUsage);

    ResourceCache(Device& device);

    Pipeline& requestPipeline(const PipelineState& state);

    void clearFrameBuffers()
    {
        state.frameBuffers.clear();
    }

    void clearShaderModules()
    {
        state.shader_modules.clear();
    }

    void clearPipelineLayouts()
    {
        state.pipeline_layouts.clear();
    }

    void clearPipelines()
    {
        state.pipelines.clear();
    }

    void clearDescriptorSetLayouts()
    {
        state.descriptor_set_layouts.clear();
    }

    void clearDescriptorPools()
    {
        state.descriptor_pools.clear();
    }

    void clearDescriptorSets()
    {
        state.descriptorSets.clear();
    }

    void clearRenderPasses()
    {
        state.render_passes.clear();
    }

    void clearSgImages()
    {
        state.sgImages.clear();
    }

private:
    static ResourceCache * cache;

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
    
    std::mutex bufferMutex;

    VkPipelineCache pipelineCache;
};
