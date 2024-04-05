#include "ResourceCache.h"
#include "Core/ResourceCachingHelper.h"
#include "Core/Device/Device.h"

ResourceCache* ResourceCache::cache = nullptr;

template<class T, class... A>
T& requestResource(Device& device, std::mutex& resourceMutex, std::unordered_map<std::size_t, T>& resources, A&... args) {
    std::lock_guard<std::mutex> guard(resourceMutex);

    auto& res = requestResource(device, resources, args...);

    return res;
}

template<class T, class... A>
T& requestResource(Device& device, const std::string& resourceName, std::mutex& resourceMutex, std::unordered_map<std::size_t, T>& resources, A&... args) {
    std::lock_guard<std::mutex> guard(resourceMutex);

    auto& res = requestResource(device, resourceName, resources, args...);

    return res;
}

ResourceCache& ResourceCache::getResourceCache() {
    assert(cache != nullptr && "Cache has not been initalized");
    return *cache;
}

void ResourceCache::initCache(Device& device) {
    assert(cache == nullptr && "Cache has been initalized");
    cache = &device.getResourceCache();
}

SgImage& ResourceCache::requestSgImage(const std::string& path, VkImageViewType viewType) {
    return requestResource(device, sgImageMutex, state.sgImages, path, viewType);
}

SgImage& ResourceCache::requestSgImage(const std::string& name, const VkExtent3D& extent, VkFormat format, VkImageUsageFlags image_usage, VmaMemoryUsage memory_usage, VkImageViewType viewType, VkSampleCountFlagBits sample_count, uint32_t mip_levels, uint32_t array_layers, VkImageCreateFlags flags) {
    // when call requestResource function,some bug will happen
    // same parameters will get different hash value
    // so we don't use requestResource function there for sgImage
    // However, I don't know why this happens
    // It happens on release mode
    // when i call log in hash_param function, it works well
    // Does it have to do with execution speed?
    // May be mutex is not working well

    
    std::lock_guard<std::mutex> mutex(sgImageMutex);
    size_t hash{};
    hash_combine(hash, name);
    hash_combine(hash, extent);
    hash_combine(hash, format);
    hash_combine(hash, image_usage);
    hash_combine(hash, memory_usage);
    hash_combine(hash, viewType);
    hash_combine(hash, sample_count);
    hash_combine(hash, mip_levels);
    hash_combine(hash, array_layers);
    hash_combine(hash, flags);
    
    if(auto res = state.sgImages.find(hash); res != state.sgImages.end()) {
        return res->second;
    }
    SgImage resource(device, name, extent, format, image_usage, memory_usage, viewType, sample_count, mip_levels, array_layers, flags);
    auto res_it = state.sgImages.emplace(hash, std::move(resource));
    return res_it.first->second;
    //return requestResource(device, sgImageMutex, state.sgImages, name, extent, format, image_usage, memory_usage, viewType, sample_count, mip_levels, array_layers, flags);
}

RenderPass& ResourceCache::requestRenderPass(const std::vector<Attachment>&  attachments,
                                             const std::vector<SubpassInfo>& subpasses) {
    return requestResource(device, renderPassMutex, state.render_passes, attachments, subpasses);
}

DescriptorLayout& ResourceCache::requestDescriptorLayout(std::vector<Shader>& shaders) {
    return requestResource(device, descriptorLayoutMutex, state.descriptor_set_layouts, shaders);
}
DescriptorLayout& ResourceCache::requestDescriptorLayout(uint32_t setIdx, const std::vector<ShaderResource>& shaderResources) {
    return requestResource(device, descriptorLayoutMutex, state.descriptor_set_layouts, setIdx, shaderResources);
}

FrameBuffer& ResourceCache::requestFrameBuffer(RenderTarget& renderTarget, RenderPass& renderPass) {
    return requestResource(device, frameBufferMutex, state.frameBuffers, renderTarget, renderPass);
}

DescriptorSet& ResourceCache::requestDescriptorSet(const DescriptorLayout&                                         descriptorSetLayout,
                                                   DescriptorPool&                                                 descriptorPool,
                                                   const BindingMap<VkDescriptorBufferInfo>&                       bufferInfos,
                                                   const BindingMap<VkDescriptorImageInfo>&                        imageInfos,
                                                   const BindingMap<VkWriteDescriptorSetAccelerationStructureKHR>& accelerations) {
    return requestResource(device, descriptorSetMutex, state.descriptorSets, descriptorSetLayout, descriptorPool, bufferInfos, imageInfos, accelerations);
}

DescriptorPool& ResourceCache::requestDescriptorPool(const DescriptorLayout& layout, uint32_t poolSize) {

    return requestResource(device, descriptorPoolMutex, state.descriptor_pools, layout, poolSize);
}

Buffer& ResourceCache::requestNamedBuffer(const std::string& name, uint64_t bufferSize, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage) {
    return requestResource(device, name, bufferMutex, state.buffers, bufferSize, bufferUsage, memoryUsage);
}

ResourceCache::ResourceCache(Device& device) : device(device) {
}

Pipeline& ResourceCache::requestPipeline(const PipelineState& pipelineState) {
    return requestResource(device, pipelineMutex, state.pipelines, pipelineState);
}

PipelineLayout& ResourceCache::requestPipelineLayout(std::vector<Shader>& shaders) {
    return requestResource(device, pipelineLayoutMutex, state.pipeline_layouts, shaders);
}