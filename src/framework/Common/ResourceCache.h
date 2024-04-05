#pragma once

#include <unordered_map>

#include "Core/Shader/Shader.h"
#include "Core/RenderPass.h"

#define GLM_ENABLE_EXPERIMENTAL

#include <gtx/hash.hpp>

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

struct ResourceCacheState {
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


template<typename T>
inline void hash_param_log(size_t& seed, const T& value) {
    // hash_combine(seed, value);
}

template<>
inline void hash_param_log<std::string>(size_t& seed, const std::string& value) {
     std::size_t hash = std::hash<std::string>{}(value);
   // LOGI("Hashing string {0} with value {1}", value, hash);
}

template<>
inline void hash_param_log<VkExtent3D>(size_t& seed, const VkExtent3D & value) {
  std::size_t hash = std::hash<VkExtent3D>{}(value);
    // std::size_t hash;
    hash_combine(hash, value);
    LOGI("Hashing VkExtent3D with value {0}",hash);
}

template<>
inline void hash_param_log<VkFormat>(size_t& seed, const VkFormat& value) {
    std::size_t hash = std::hash<VkFormat>{}(value);
    //LOGI("Hashing VkFormat with value {0}", hash);
}


template<typename T, typename... Args>
inline void hash_param_log(size_t& seed, const T& first_arg, const Args&... args) {
    hash_param_log(seed, first_arg);
    //
   hash_param_log(seed, args...);
}

// template<typename T>
// inline std::string get_str(const T& value) {
//     return "";
// }
//
// template<>
// inline std::string get_str<std::string>(const std::string& value) {
//     return value;
// }
//
// template<>
// inline std::string get_str<VkExtent3D>(const VkExtent3D& value) {
//     return std::to_string(value.width) + std::to_string(value.height) + std::to_string(value.depth);
// }
//
// template<>
// inline std::string get_str<VkFormat>(const VkFormat& value) {
//     return std::to_string(value);
// }
//
// template<>
// inline std::string get_str<VkImageViewType>(const VkImageViewType& value) {
//     return std::to_string(value);
// }
//
// template<>
// inline std::string get_str<VkImageUsageFlags>(const VkImageUsageFlags& value) {
//     return std::to_string(value);
// }
//
// template<>
// inline std::string get_str<VmaMemoryUsage>(const VmaMemoryUsage& value) {
//     return std::to_string(value);
// }
//
// template<>
// inline std::string get_str<VkSampleCountFlagBits>(const VkSampleCountFlagBits& value) {
//     return std::to_string(value);
// }
//
template<typename  T>
inline std::string get_str(size_t & seed,const T& value) {
    hash_combine(seed, value);
    return std::to_string(seed);
}

template<typename T, typename... Args>
inline std::string get_str(size_t & seed,const T& first_arg, const Args&... args) {
    return get_str(seed, first_arg) + " "+get_str(seed,args...);
}

// template<typename T, typename... Args>
// inline void hash_param_log(size_t& seed, const T& first_arg, const Args&... args) {
//     hash_param_log(seed, first_arg);
//     //
//     hash_param_log(seed, args...);
// }


template<class T, class... A>
T& requestResource(Device& device, std::unordered_map<std::size_t, T>& resources, A&... args) {
    std::size_t hash{0U};
    hash_param(hash, args...);
    auto res = resources.find(hash);
    if (res != resources.end()) {
        return res->second;
    }

    if(typeid(T) == typeid(SgImage)){
        hash_param_log(hash,args...);
    }

    hash_param_log(hash,args...);

    size_t hash_new = 0;
  std::string str = get_str(hash_new,args...);

    // size_t hash_new = 0;
    // hash_param(hash_new, args...);
  LOGI("Creating resource of type {0} with hash {1} {2} {3}", typeid(T).name(), hash, str, hash_new);

    if(resources.contains(hash)) {
        exit(-1);
    }
    T    resource(device, args...);
    auto res_it = resources.emplace(hash, std::move(resource));


    res = res_it.first;
    return res->second;

    // return resources[hash]->second;
}








template<class T, class... A>
T& requestResource(Device& device, const std::string& name, std::unordered_map<std::size_t, T>& resources, A&... args) {
    std::size_t hash{0U};
    hash_param(hash, name);
    hash_param(hash, args...);
    auto res = resources.find(hash);
    if (res != resources.end()) {
        return res->second;
    }

    T    resource(device, args...);
    auto res_it = resources.emplace(hash, std::move(resource));

    res = res_it.first;
    return res->second;

    // return resources[hash]->second;
}

class ResourceCache {
public:
    // std::unordered_map<Rende>
    ~ResourceCache() = default;

    static ResourceCache& getResourceCache();

    static void initCache(Device& device);

    SgImage& requestSgImage(const std::string& path, VkImageViewType viewType);

    SgImage& requestSgImage(const std::string&    name,
                            const VkExtent3D&     extent,
                            VkFormat              format,
                            VkImageUsageFlags     image_usage,
                            VmaMemoryUsage        memory_usage,
                            VkImageViewType       viewType,
                            VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT,
                            uint32_t              mip_levels   = 1,
                            uint32_t              array_layers = 1,
                            VkImageCreateFlags    flags        = 0);

    RenderPass& requestRenderPass(const std::vector<Attachment>&  attachments,
                                  const std::vector<SubpassInfo>& subpasses);

    DescriptorLayout& requestDescriptorLayout(std::vector<Shader>& shaders);
    DescriptorLayout& requestDescriptorLayout(uint32_t setIdx, const std::vector<ShaderResource>& shaderResources);

    PipelineLayout& requestPipelineLayout(std::vector<Shader>& shaders);

    FrameBuffer& requestFrameBuffer(RenderTarget& renderTarget, RenderPass& renderPass);

    DescriptorSet& requestDescriptorSet(const DescriptorLayout&                                         descriptorSetLayout,
                                        DescriptorPool&                                                 descriptorPool,
                                        const BindingMap<VkDescriptorBufferInfo>&                       bufferInfos,
                                        const BindingMap<VkDescriptorImageInfo>&                        imageInfos,
                                        const BindingMap<VkWriteDescriptorSetAccelerationStructureKHR>& accelerations);

    DescriptorPool& requestDescriptorPool(const DescriptorLayout& layout,
                                          uint32_t                poolSize = DescriptorPool::MAX_SETS_PER_POOL);

    Buffer& requestNamedBuffer(const std::string& name, uint64_t bufferSize, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage);

    ResourceCache(Device& device);

    Pipeline& requestPipeline(const PipelineState& state);

    void clearFrameBuffers() {
        state.frameBuffers.clear();
    }

    void clearShaderModules() {
        state.shader_modules.clear();
    }

    void clearPipelineLayouts() {
        state.pipeline_layouts.clear();
    }

    void clearPipelines() {
        state.pipelines.clear();
    }

    void clearDescriptorSetLayouts() {
        state.descriptor_set_layouts.clear();
    }

    void clearDescriptorPools() {
        state.descriptor_pools.clear();
    }

    void clearDescriptorSets() {
        state.descriptorSets.clear();
    }

    void clearRenderPasses() {
        state.render_passes.clear();
    }

    void clearSgImages() {
        state.sgImages.clear();
    }

private:
    static ResourceCache* cache;

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