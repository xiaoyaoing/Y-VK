#pragma once
#include <unordered_map>

#include "Shader.h"
#include "RenderPass.h"

#include <glm/gtx/hash.hpp>

#include "FrameBuffer.h"
#include "Pipeline.h"
#include "RenderTarget.h"
#include "Descriptor/DescriptorLayout.h"


//A resource cache  system
//Mainly from vulkan-samples

namespace std
{
    template <class T>
    inline void hash_combine(size_t& seed, const T& v)
    {
        std::hash<T> hasher;
        glm::detail::hash_combine(seed, hasher(v));
    }


    template <class T>
    inline void hash_combine(size_t& seed, const std::vector<T>& vars)
    {
        for (const auto& var : vars)
        {
            std::hash<T> hasher;
            glm::detail::hash_combine(seed, hasher(var));
        }
    }

    // template <std::vector<Shader> >>
    // inline void hash_combine(size_t& seed, const std::vector<T>& vars)
    // {
    //     for (const auto& var : vars)
    //     {
    //         std::hash<T> hasher;
    //         glm::detail::hash_combine(seed, hasher(var));
    //     }
    // }


    template <>
    struct hash<Shader>
    {
        std::size_t operator()(const Shader& shader) const
        {
            return shader.getId();
        }

        std::size_t operator()(const std::vector<Shader>& shader) const
        {
            return 0;
        }
    };


    template <>
    struct hash<Attachment>
    {
        std::size_t operator()(const Attachment& attachment) const
        {
            size_t hash{0U};

            hash_combine(hash, attachment.format);
            hash_combine(hash, attachment.initial_layout);
            hash_combine(hash, attachment.samples);
            hash_combine(hash, attachment.usage);

            return hash;
        }
    };


    template <>
    struct hash<SubpassInfo>
    {
        std::size_t operator()(const SubpassInfo& subpassInfo) const
        {
            size_t hash{0U};

            hash_combine(hash, subpassInfo.debugName);
            hash_combine(hash, subpassInfo.inputAttachments);
            hash_combine(hash, subpassInfo.outputAttachments);
            hash_combine(hash, subpassInfo.colorResolveAttachments);


            hash_combine(hash, subpassInfo.disableDepthStencilAttachment);
            hash_combine(hash, subpassInfo.depthStencilResolveAttachment);
            hash_combine(hash, subpassInfo.depthStencilResolveMode);

            return hash;
        }
    };


    // template <>
    // inline void hash_combine(size_t& seed, const std::vector<Shader*>& shaders)
    // {
    //     for (auto shader : shaders)
    //         glm::detail::hash_combine(seed, shader->getId());
    // }
}

template <typename T>
inline void hash_param(size_t& seed, const T& value)
{
    hash_combine(seed, value);
}


template <typename T, typename... Args>
inline void hash_param(size_t& seed, const T& first_arg, const Args&... args)
{
    hash_param(seed, first_arg);

    hash_param(seed, args...);
}


struct ResourceCacheState
{
    std::unordered_map<std::size_t, Shader> shader_modules;

    // std::unordered_map<std::size_t, PipelineLayout> pipeline_layouts;
    //
    std::unordered_map<std::size_t, DescriptorLayout> descriptor_set_layouts;
    //
    // std::unordered_map<std::size_t, DescriptorPool> descriptor_pools;

    std::unordered_map<std::size_t, RenderPass> render_passes;
    //
    // std::unordered_map<std::size_t, GraphicsPipeline> graphics_pipelines;
    //
    // std::unordered_map<std::size_t, ComputePipeline> compute_pipelines;
    //
    // std::unordered_map<std::size_t, DescriptorSet> descriptor_sets;
    //
    std::unordered_map<std::size_t, FrameBuffer> framebuffers;
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

    RenderPass& requestRenderPass(const std::vector<Attachment>& attachments,
                                  const std::vector<SubpassInfo>& subpasses);

    DescriptorLayout& requestDescriptorLayout(std::vector<Shader>& shaders);

    FrameBuffer& requestFrameBuffer(RenderTarget& renderTarget, RenderPass& renderPass);

    ResourceCache(Device& device);

    Pipeline& requestPipeline(const PipelineState& state);

private:
    static std::unique_ptr<ResourceCache> cache;

    ResourceCacheState state;

    Device& device;

    std::mutex renderPassMutex;

    std::mutex descriptorMutex;

    std::mutex frameBufferMutex;

    std::mutex pipelineMutex;

    VkPipelineCache pipelineCache;
};

std::unique_ptr<ResourceCache> ResourceCache::cache = nullptr;
