#include "ResourceCache.h"

template <class T, class... A>
T& requestResource(Device& device, std::mutex& resourceMutex, std::unordered_map<std::size_t, T>& resources, A&... args)
{
    std::lock_guard<std::mutex> guard(resourceMutex);

    auto& res = requestResource(device, resources, args...);

    return res;
}

ResourceCache& ResourceCache::getResourceCache()
{
    assert(cache !=nullptr && "Cache has not been initalized");
    return *cache;
}

void ResourceCache::initCache(Device& device)
{
    assert(cache == nullptr && "Cache has been initalized");
    cache = std::make_unique<ResourceCache>(device);
}

RenderPass& ResourceCache::requestRenderPass(const std::vector<Attachment>& attachments,
                                             const std::vector<SubpassInfo>& subpasses)
{
    return requestResource(device, renderPassMutex, state.render_passes, attachments, subpasses);
}

DescriptorLayout& ResourceCache::requestDescriptorLayout(std::vector<Shader>& shaders)
{
    // DescriptorLayout d(device);
    //  return d;
    return requestResource(device, descriptorMutex, state.descriptor_set_layouts, shaders);
}

FrameBuffer& ResourceCache::requestFrameBuffer(RenderTarget& renderTarget, RenderPass& renderPass)
{
    return requestResource(device, frameBufferMutex, state.framebuffers, renderTarget, renderPass);
}

ResourceCache::ResourceCache(Device& device): device(device)
{
}
