#pragma  once

#include <glm/gtx/hash.hpp>

#include "FrameBuffer.h"
#include "Pipeline.h"
#include "RenderTarget.h"
#include "Descriptor/DescriptorLayout.h"
#include "Descriptor/DescriptorPool.h"
#include "RenderPass.h"


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

template <class T>
inline void hash_combine(size_t& seed, const BindingMap<T>& vars)
{
    for (const auto& bindingIt : vars)
    {
        hash_combine(seed, bindingIt.first);
        for (const auto& bindingElement : bindingIt.second)
        {
            hash_combine(seed, bindingElement.first);
            hash_combine(seed, bindingElement.second);
        }
    }
}


namespace std
{
    // template <std::vector<Shader> >>
    // inline void hash_combine(size_t& seed, const std::vector<T>& vars)
    // {
    //     for (const auto& var : vars)
    //     {
    //         std::hash<T> hasher;
    //         glm::detail::hash_combine(seed, hasher(var));
    //     }
    // }


    //    template <>
    // struct hash<>
    // {
    // 	std::size_t operator()(const & ) const
    // 	{
    // 		size_t hash{0U};
    //
    // 		return hash;
    // 	}
    // };


    template <>
    struct hash<VkExtent3D>
    {
        std::size_t operator()(const VkExtent3D extent3d) const
        {
            std::size_t result;
            hash_combine(result, extent3d.width);
            hash_combine(result, extent3d.height);
            hash_combine(result, extent3d.width);
            return result;
        }
    };


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
    struct hash<RenderTarget>
    {
        std::size_t operator()(const RenderTarget& renderTarget) const
        {
            std::size_t res = 0;
            for (auto& attachment : renderTarget.getHwTextures())
            {
                hash_combine(res, attachment->getVkImage().getHandle());
                hash_combine(res, attachment->getVkImageView().getHandle());
            }
            return res;
        }
    };

    template <>
    struct hash<RenderPass>
    {
        std::size_t operator()(const RenderPass& renderPass) const
        {
            std::size_t res = 0;
            hash_combine(res, renderPass.getHandle());
            return res;
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


    template <>
    struct hash<DescriptorLayout>
    {
        std::size_t operator()(const DescriptorLayout& layout) const
        {
            size_t hash{0U};

            hash_combine(hash, layout.getBindings());
            hash_combine(hash, layout.getHandle());
            return hash;
        }
    };


    template <>
    struct hash<DescriptorPool>
    {
        std::size_t operator()(const DescriptorPool& pool) const
        {
            size_t hash{0U};

            hash_combine(hash, pool.getHandle());
            return hash;
        }
    };


    template <>
    struct hash<VkDescriptorSetLayoutBinding>
    {
        std::size_t operator()(const VkDescriptorSetLayoutBinding& binding) const
        {
            size_t hash{0U};
            hash_combine(hash, binding.binding);
            hash_combine(hash, binding.descriptorCount);
            hash_combine(hash, binding.descriptorType);

            return hash;
        }
    };

    template <>
    struct hash<VkDescriptorBufferInfo>
    {
        std::size_t operator()(const VkDescriptorBufferInfo& descriptor_buffer_info) const
        {
            std::size_t result = 0;

            hash_combine(result, descriptor_buffer_info.buffer);
            hash_combine(result, descriptor_buffer_info.range);
            hash_combine(result, descriptor_buffer_info.offset);

            return result;
        }
    };

    template <>
    struct hash<VkDescriptorImageInfo>
    {
        std::size_t operator()(const VkDescriptorImageInfo& descriptor_image_info) const
        {
            std::size_t result = 0;

            hash_combine(result, descriptor_image_info.imageView);
            hash_combine(
                result, static_cast<std::underlying_type<VkImageLayout>::type>(descriptor_image_info.imageLayout));
            hash_combine(result, descriptor_image_info.sampler);

            return result;
        }
    };

    template <>
    struct hash<VkWriteDescriptorSet>
    {
        std::size_t operator()(const VkWriteDescriptorSet& write_descriptor_set) const
        {
            std::size_t result = 0;

            hash_combine(result, write_descriptor_set.dstSet);
            hash_combine(result, write_descriptor_set.dstBinding);
            hash_combine(result, write_descriptor_set.dstArrayElement);
            hash_combine(result, write_descriptor_set.descriptorCount);
            hash_combine(result, write_descriptor_set.descriptorType);

            switch (write_descriptor_set.descriptorType)
            {
            case VK_DESCRIPTOR_TYPE_SAMPLER:
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                for (uint32_t i = 0; i < write_descriptor_set.descriptorCount; i++)
                {
                    hash_combine(result, write_descriptor_set.pImageInfo[i]);
                }
                break;

            case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                for (uint32_t i = 0; i < write_descriptor_set.descriptorCount; i++)
                {
                    hash_combine(result, write_descriptor_set.pTexelBufferView[i]);
                }
                break;

            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                for (uint32_t i = 0; i < write_descriptor_set.descriptorCount; i++)
                {
                    hash_combine(result, write_descriptor_set.pBufferInfo[i]);
                }
                break;

            default:
                // Not implemented
                break;
            }

            return result;
        }
    };
}


template <typename T>
inline void hash_param(size_t& seed, const T& value)
{
    hash_combine(seed, value);
}

// template<typename T>
// inline void hash_param(size_t &seed, const BindingMap<T> &vars) {
//     // for (const auto &var: vars) {
//     //     std::hash<T> hasher;
//     //     for (auto &bindingVar: vars) {
//     //         hash_combine(seed, bindingVar.first);
//     //         hash_combine(seed, bindingVar.second);
//     //     }
//     // }
// }


template <typename T, typename... Args>
inline void hash_param(size_t& seed, const T& first_arg, const Args&... args)
{
    hash_param(seed, first_arg);

    hash_param(seed, args...);
}
