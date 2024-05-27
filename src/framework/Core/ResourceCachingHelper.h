#pragma once

#include <glm/gtx/hash.hpp>

#include "PipelineState.h"
#include "RenderTarget.h"
#include "Core/Descriptor/DescriptorLayout.h"
#include "Core/Descriptor/DescriptorPool.h"
#include "RenderPass.h"

template<class>
inline constexpr bool is_vector_v = false;
template<class T, class A>
inline constexpr bool is_vector_v<std::vector<T, A>> = true;

template<class T>
inline void hash_combine(size_t& seed, const T& v) {
    std::hash<T> hasher;
    glm::detail::hash_combine(seed, hasher(v));
}

template<class T>
inline void hash_combine(size_t& seed, const std::vector<T>& vars) {
    for (const auto& var : vars) {
        std::hash<T> hasher;
        glm::detail::hash_combine(seed, hasher(var));
    }
}

template<class T>
inline void hash_combine(size_t& seed, const BindingMap<T>& vars) {
    for (const auto& bindingIt : vars) {
        hash_combine(seed, bindingIt.first);
        for (const auto& bindingElement : bindingIt.second) {
            hash_combine(seed, bindingElement.first);
            hash_combine(seed, bindingElement.second);
        }
    }
}

namespace std {
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
    template<>
    struct hash<VkWriteDescriptorSet> {
        std::size_t operator()(const VkWriteDescriptorSet& write_descriptor_set) const {
            std::size_t result = 0;

            hash_combine(result, write_descriptor_set.dstSet);
            hash_combine(result, write_descriptor_set.dstBinding);
            hash_combine(result, write_descriptor_set.dstArrayElement);
            hash_combine(result, write_descriptor_set.descriptorCount);
            hash_combine(result, write_descriptor_set.descriptorType);

            switch (write_descriptor_set.descriptorType) {
                case VK_DESCRIPTOR_TYPE_SAMPLER:
                case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                    for (uint32_t i = 0; i < write_descriptor_set.descriptorCount; i++) {
                        hash_combine(result, write_descriptor_set.pImageInfo[i]);
                    }
                    break;

                case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
                case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
                    for (uint32_t i = 0; i < write_descriptor_set.descriptorCount; i++) {
                        hash_combine(result, write_descriptor_set.pTexelBufferView[i]);
                    }
                    break;

                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                    for (uint32_t i = 0; i < write_descriptor_set.descriptorCount; i++) {
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

    template<>
    struct hash<VkExtent3D> {
        std::size_t operator()(const VkExtent3D extent3d) const noexcept {
            std::size_t result;
            hash_combine(result, extent3d.width);
            hash_combine(result, extent3d.height);
            hash_combine(result, extent3d.width);
            return result;
        }
    };

    template<>
    struct hash<ShaderResource> {
        std::size_t operator()(const ShaderResource& shader_resource) const {
            std::size_t result = 0;

            if (shader_resource.type == ShaderResourceType::Input ||
                shader_resource.type == ShaderResourceType::Output ||
                shader_resource.type == ShaderResourceType::PushConstant ||
                shader_resource.type == ShaderResourceType::SpecializationConstant) {
                return result;
            }

            hash_combine(result, shader_resource.set);
            hash_combine(result, shader_resource.binding);
            hash_combine(result, static_cast<std::underlying_type<ShaderResourceType>::type>(shader_resource.type));
            hash_combine(result, shader_resource.name);
            hash_combine(result, shader_resource.location);
            hash_combine(result, shader_resource.arraySize);
            hash_combine(result, shader_resource.stages);
            hash_combine(result, shader_resource.offset);
            hash_combine(result, shader_resource.size);

            return result;
        }
    };

    template<>
    struct hash<Shader> {
        std::size_t operator()(const Shader& shader) const {
            return shader.getId();
        }

        std::size_t operator()(const std::vector<Shader>& shader) const {
            return 0;
        }
    };

    template<>
    struct hash<RenderTarget> {
        std::size_t operator()(const RenderTarget& renderTarget) const {
            std::size_t res = 0;
            for (auto& attachment : renderTarget.getHwTextures()) {
                hash_combine(res, attachment->getVkImage().getHandle());
                hash_combine(res, attachment->getVkImageView().getHandle());
            }
            return res;
        }
    };

    template<>
    struct hash<RenderPass> {
        std::size_t operator()(const RenderPass& renderPass) const {
            std::size_t res = 0;
            hash_combine(res, renderPass.getHandle());
            return res;
        }
    };

    template<>
    struct hash<Attachment> {
        std::size_t operator()(const Attachment& attachment) const {
            size_t hash{0U};

            hash_combine(hash, attachment.format);
            hash_combine(hash, attachment.initial_layout);
            hash_combine(hash, attachment.samples);
            hash_combine(hash, attachment.usage);
            hash_combine(hash, attachment.loadOp);
            hash_combine(hash, attachment.storeOp);

            return hash;
        }
    };

    template<>
    struct hash<SubpassInfo> {
        std::size_t operator()(const SubpassInfo& subpassInfo) const {
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

    template<>
    struct hash<DescriptorLayout> {
        std::size_t operator()(const DescriptorLayout& layout) const {
            size_t hash{0U};

            hash_combine(hash, layout.getBindings());
            hash_combine(hash, layout.getHandle());
            return hash;
        }
    };

    template<>
    struct hash<DescriptorPool> {
        std::size_t operator()(const DescriptorPool& pool) const {
            size_t hash{0U};

            hash_combine(hash, pool.getDescriptorLayout().getHandle());
            return hash;
        }
    };

    template<>
    struct hash<VkDescriptorSetLayoutBinding> {
        std::size_t operator()(const VkDescriptorSetLayoutBinding& binding) const {
            size_t hash{0U};
            hash_combine(hash, binding.binding);
            hash_combine(hash, binding.descriptorCount);
            hash_combine(hash, binding.descriptorType);

            return hash;
        }
    };

    template<>
    struct hash<VkDescriptorBufferInfo> {
        std::size_t operator()(const VkDescriptorBufferInfo& descriptor_buffer_info) const {
            std::size_t result = 0;

            hash_combine(result, descriptor_buffer_info.buffer);
            hash_combine(result, descriptor_buffer_info.range);
            hash_combine(result, descriptor_buffer_info.offset);

            return result;
        }
    };

    template<>
    struct hash<VkDescriptorImageInfo> {
        std::size_t operator()(const VkDescriptorImageInfo& descriptor_image_info) const {
            std::size_t result = 0;

            hash_combine(result, descriptor_image_info.imageView);
            hash_combine(
                result, static_cast<std::underlying_type<VkImageLayout>::type>(descriptor_image_info.imageLayout));
            hash_combine(result, descriptor_image_info.sampler);

            return result;
        }
    };

    template<>
    struct hash<VkWriteDescriptorSetAccelerationStructureKHR> {
        std::size_t operator()(const VkWriteDescriptorSetAccelerationStructureKHR& write_descriptor_set) const {
            std::size_t result = 0;

            hash_combine(result, write_descriptor_set.accelerationStructureCount);
            for (size_t i = 0; i < write_descriptor_set.accelerationStructureCount; i++) {
                hash_combine(result, write_descriptor_set.pAccelerationStructures[i]);
            }
            return result;
        }
    };

    template<>
    struct hash<SpecializationConstantState> {
        // std::size_t operator()(const SpecializationConstantState &specialization_constant_state) const
        // {
        //     std::size_t result = 0;
        //
        //     for (auto constants : specialization_constant_state.)
        //     {
        //        hash_combine(result, constants.first);
        //         for (const auto data : constants.second)
        //         {
        //            hash_combine(result, data);
        //         }
        //     }
        //
        //     return result;
        // }
    };

    template<>
    struct hash<VkVertexInputAttributeDescription> {
        std::size_t operator()(const VkVertexInputAttributeDescription& vertex_attrib) const {
            std::size_t result = 0;

            hash_combine(result, vertex_attrib.binding);
            hash_combine(result, static_cast<std::underlying_type<VkFormat>::type>(vertex_attrib.format));
            hash_combine(result, vertex_attrib.location);
            hash_combine(result, vertex_attrib.offset);

            return result;
        }
    };

    template<>
    struct hash<VkVertexInputBindingDescription> {
        std::size_t operator()(const VkVertexInputBindingDescription& vertex_binding) const {
            std::size_t result = 0;

            hash_combine(result, vertex_binding.binding);
            hash_combine(result, static_cast<std::underlying_type<VkVertexInputRate>::type>(vertex_binding.inputRate));
            hash_combine(result, vertex_binding.stride);

            return result;
        }
    };

    template<>
    struct hash<ColorBlendAttachmentState> {
        std::size_t operator()(const ColorBlendAttachmentState& colorBlendAttachment) const {
            std::size_t result = 0;

            hash_combine(result, static_cast<std::underlying_type<VkBlendOp>::type>(colorBlendAttachment.alphaBlendOp));
            hash_combine(result, colorBlendAttachment.blendEnable);
            hash_combine(result, static_cast<std::underlying_type<VkBlendOp>::type>(colorBlendAttachment.colorBlendOp));
            hash_combine(result, colorBlendAttachment.colorWriteMask);
            hash_combine(
                result,
                static_cast<std::underlying_type<VkBlendFactor>::type>(colorBlendAttachment.dstAlphaBlendFactor));
            hash_combine(
                result,
                static_cast<std::underlying_type<VkBlendFactor>::type>(colorBlendAttachment.dstColorBlendFactor));
            hash_combine(
                result,
                static_cast<std::underlying_type<VkBlendFactor>::type>(colorBlendAttachment.srcAlphaBlendFactor));
            hash_combine(
                result,
                static_cast<std::underlying_type<VkBlendFactor>::type>(colorBlendAttachment.srcColorBlendFactor));

            return result;
        }
    };

    template<>
    struct hash<StencilOpState> {
        std::size_t operator()(const StencilOpState& stencil) const {
            std::size_t result = 0;

            hash_combine(result, static_cast<std::underlying_type<VkCompareOp>::type>(stencil.compareOp));
            hash_combine(result, static_cast<std::underlying_type<VkStencilOp>::type>(stencil.depthFailOp));
            hash_combine(result, static_cast<std::underlying_type<VkStencilOp>::type>(stencil.failOp));
            hash_combine(result, static_cast<std::underlying_type<VkStencilOp>::type>(stencil.passOp));

            return result;
        }
    };

    template<>
    struct hash<RTPipelineSettings> {
        std::size_t operator()(const RTPipelineSettings& rTPipelineSettings) const {
            std::size_t result = 0;

            // hash_combine(result, rTPipelineSettings.debugName);
            // hash_combine(result, rTPipelineSettings.maxRecursionDepth);
            // hash_combine(result, rTPipelineSettings.raygenShader);
            // hash_combine(result, rTPipelineSettings.missShaders);
            // hash_combine(result, rTPipelineSettings.hitGroups);
            // hash_combine(result, rTPipelineSettings.callableShaders);
            // hash_combine(result, rTPipelineSettings.width);
            // hash_combine(result, rTPipelineSettings.height);
            // hash_combine(result, rTPipelineSettings.depth);
            // hash_combine(result, rTPipelineSettings.shaderBindingTable);

            return result;
        }
    };

    template<>
    struct hash<PipelineState> {
        std::size_t operator()(const PipelineState& pipelineState) const {
            std::size_t result = 0;

            hash_combine(result, pipelineState.getPipelineLayout().getHandle());

            // For graphics only
            if (auto renderPass = pipelineState.getRenderPass()) {
                hash_combine(result, renderPass->getHandle());
            }

            // fixme
            // hash_combine(result, pipelineState.getSpecializationConstantState());

            hash_combine(result, pipelineState.getSubpassIndex());

            for (auto& shaderModule : pipelineState.getPipelineLayout().getShaders()) {
                hash_combine(result, shaderModule->getId());
            }

            // VkPipelineVertexInputStateCreateInfo
            for (auto& attribute : pipelineState.getVertexInputState().attributes) {
                hash_combine(result, attribute);
            }

            for (auto& binding : pipelineState.getVertexInputState().bindings) {
                hash_combine(result, binding);
            }

            // VkPipelineInputAssemblyStateCreateInfo
            hash_combine(result, pipelineState.getInputAssemblyState().primitiveRestartEnable);
            hash_combine(
                result,
                static_cast<std::underlying_type<VkPrimitiveTopology>::type>(
                    pipelineState.getInputAssemblyState().topology));

            // VkPipelineViewportStateCreateInfo
            hash_combine(result, pipelineState.getViewportState().viewportCount);
            hash_combine(result, pipelineState.getViewportState().scissorCount);

            // VkPipelineRasterizationStateCreateInfo
            hash_combine(result, pipelineState.getRasterizationState().cullMode);
            hash_combine(result, pipelineState.getRasterizationState().depthBiasEnable);
            hash_combine(result, pipelineState.getRasterizationState().depthClampEnable);
            hash_combine(
                result,
                static_cast<std::underlying_type<VkFrontFace>::type>(
                    pipelineState.getRasterizationState().frontFace));
            hash_combine(result, static_cast<std::underlying_type<VkPolygonMode>::type>(pipelineState.getRasterizationState().polygonMode));
            hash_combine(result, pipelineState.getRasterizationState().rasterizerDiscardEnable);
            hash_combine(result, pipelineState.getRasterizationState().pNext);

            // VkPipelineMultisampleStateCreateInfo
            hash_combine(result, pipelineState.getMultisampleState().alphaToCoverageEnable);
            hash_combine(result, pipelineState.getMultisampleState().alphaToOneEnable);
            hash_combine(result, pipelineState.getMultisampleState().minSampleShading);
            hash_combine(
                result,
                static_cast<std::underlying_type<VkSampleCountFlagBits>::type>(
                    pipelineState.getMultisampleState().rasterizationSamples));
            hash_combine(result, pipelineState.getMultisampleState().sampleShadingEnable);
            hash_combine(result, pipelineState.getMultisampleState().sampleMask);

            // VkPipelineDepthStencilStateCreateInfo
            hash_combine(result, pipelineState.getDepthStencilState().back);
            hash_combine(result, pipelineState.getDepthStencilState().depthBoundsTestEnable);
            hash_combine(
                result,
                static_cast<std::underlying_type<VkCompareOp>::type>(
                    pipelineState.getDepthStencilState().depthCompareOp));
            hash_combine(result, pipelineState.getDepthStencilState().depthTestEnable);
            hash_combine(result, pipelineState.getDepthStencilState().depthWriteEnable);
            hash_combine(result, pipelineState.getDepthStencilState().front);
            hash_combine(result, pipelineState.getDepthStencilState().stencilTestEnable);

            // VkPipelineColorBlendStateCreateInfo
            hash_combine(
                result,
                static_cast<std::underlying_type<VkLogicOp>::type>(
                    pipelineState.getColorBlendState().logicOp));
            hash_combine(result, pipelineState.getColorBlendState().logicOpEnable);

            for (auto& attachment : pipelineState.getColorBlendState().attachments) {
                hash_combine(result, attachment);
            }

            hash_combine(result, pipelineState.getPipelineType());

            return result;
        }
    };
}// namespace std

template<typename T>
inline void hash_param(size_t& seed, const T& value) {
    hash_combine(seed, value);
}

template<typename T, typename... Args>
inline void hash_param(size_t& seed, const T& first_arg, const Args&... args) {
    hash_param(seed, first_arg);
    // LOGI("{} {} hash", typeid(T).name(), seed);
    hash_param(seed, args...);
}