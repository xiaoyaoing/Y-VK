//
// Created by pc on 2023/8/17.
//
#pragma once

#include "Core/Vulkan.h"
#include "Core/Texture.h"

//class Texture;

namespace vkCommon {
    namespace initializers {
        inline VkDescriptorPoolSize descriptorPoolSize(VkDescriptorType type, uint32_t descriptorCount) {
            return {type, descriptorCount};
        }

         VkViewport viewport(
            float width,
            float height,
            float minDepth,
            float maxDepth) ;

        inline VkRect2D rect2D(
            int32_t width,
            int32_t height,
            int32_t offsetX,
            int32_t offsetY) {
            VkRect2D rect2D{};
            rect2D.extent.width  = width;
            rect2D.extent.height = height;
            rect2D.offset.x      = offsetX;
            rect2D.offset.y      = offsetY;
            return rect2D;
        }

        inline VkDescriptorImageInfo descriptorImageInfo(Texture& texture) {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.sampler     = texture.sampler->getHandle();
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView   = texture.image->getVkImageView().getHandle();
            return imageInfo;
        }

        inline VkPushConstantRange pushConstantRange(
            VkShaderStageFlags stageFlags,
            uint32_t           size,
            uint32_t           offset) {
            VkPushConstantRange pushConstantRange{};
            pushConstantRange.stageFlags = stageFlags;
            pushConstantRange.offset     = offset;
            pushConstantRange.size       = size;
            return pushConstantRange;
        }

        inline VkPipelineLayoutCreateInfo pipelineLayout() {
            return {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        }

        inline VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(
            std::vector<VkDescriptorSetLayout>& pSetLayouts) {
            VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
            pipelineLayoutCreateInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutCreateInfo.setLayoutCount = pSetLayouts.size();
            pipelineLayoutCreateInfo.pSetLayouts    = pSetLayouts.data();
            return pipelineLayoutCreateInfo;
        }

        inline VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(
            VkDescriptorSetLayout pSetLayout) {
            VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
            pipelineLayoutCreateInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutCreateInfo.setLayoutCount = 1;
            pipelineLayoutCreateInfo.pSetLayouts    = &pSetLayout;
            return pipelineLayoutCreateInfo;
        }

        inline VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(
            VkPrimitiveTopology                     topology,
            VkPipelineInputAssemblyStateCreateFlags flags,
            VkBool32                                primitiveRestartEnable) {
            VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo{};
            pipelineInputAssemblyStateCreateInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            pipelineInputAssemblyStateCreateInfo.topology               = topology;
            pipelineInputAssemblyStateCreateInfo.flags                  = flags;
            pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = primitiveRestartEnable;
            return pipelineInputAssemblyStateCreateInfo;
        }

        inline VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(
            VkPolygonMode                           polygonMode,
            VkCullModeFlags                         cullMode,
            VkFrontFace                             frontFace,
            VkPipelineRasterizationStateCreateFlags flags = 0) {
            VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo{};
            pipelineRasterizationStateCreateInfo.sType            = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            pipelineRasterizationStateCreateInfo.polygonMode      = polygonMode;
            pipelineRasterizationStateCreateInfo.cullMode         = cullMode;
            pipelineRasterizationStateCreateInfo.frontFace        = frontFace;
            pipelineRasterizationStateCreateInfo.flags            = flags;
            pipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
            pipelineRasterizationStateCreateInfo.lineWidth        = 1.0f;
            return pipelineRasterizationStateCreateInfo;
        }

        inline VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(
            VkColorComponentFlags colorWriteMask,
            VkBool32              blendEnable) {
            VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState{};
            pipelineColorBlendAttachmentState.colorWriteMask = colorWriteMask;
            pipelineColorBlendAttachmentState.blendEnable    = blendEnable;
            return pipelineColorBlendAttachmentState;
        }

        inline VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(
            uint32_t                                   attachmentCount,
            const VkPipelineColorBlendAttachmentState* pAttachments) {
            VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo{};
            pipelineColorBlendStateCreateInfo.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            pipelineColorBlendStateCreateInfo.attachmentCount = attachmentCount;
            pipelineColorBlendStateCreateInfo.pAttachments    = pAttachments;
            return pipelineColorBlendStateCreateInfo;
        }

        inline VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo(
            VkBool32    depthTestEnable,
            VkBool32    depthWriteEnable,
            VkCompareOp depthCompareOp) {
            VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo{};
            pipelineDepthStencilStateCreateInfo.sType            = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            pipelineDepthStencilStateCreateInfo.depthTestEnable  = depthTestEnable;
            pipelineDepthStencilStateCreateInfo.depthWriteEnable = depthWriteEnable;
            pipelineDepthStencilStateCreateInfo.depthCompareOp   = depthCompareOp;
            pipelineDepthStencilStateCreateInfo.back.compareOp   = VK_COMPARE_OP_ALWAYS;
            return pipelineDepthStencilStateCreateInfo;
        }

        inline VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(
            uint32_t                           viewportCount,
            uint32_t                           scissorCount,
            VkPipelineViewportStateCreateFlags flags = 0) {
            VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo{};
            pipelineViewportStateCreateInfo.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            pipelineViewportStateCreateInfo.viewportCount = viewportCount;
            pipelineViewportStateCreateInfo.scissorCount  = scissorCount;
            pipelineViewportStateCreateInfo.flags         = flags;
            return pipelineViewportStateCreateInfo;
        }

        inline VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo(
            VkSampleCountFlagBits                 rasterizationSamples,
            VkPipelineMultisampleStateCreateFlags flags = 0) {
            VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo{};
            pipelineMultisampleStateCreateInfo.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            pipelineMultisampleStateCreateInfo.rasterizationSamples = rasterizationSamples;
            pipelineMultisampleStateCreateInfo.flags                = flags;
            return pipelineMultisampleStateCreateInfo;
        }

        inline VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(
            const VkDynamicState*             pDynamicStates,
            uint32_t                          dynamicStateCount,
            VkPipelineDynamicStateCreateFlags flags = 0) {
            VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};
            pipelineDynamicStateCreateInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            pipelineDynamicStateCreateInfo.pDynamicStates    = pDynamicStates;
            pipelineDynamicStateCreateInfo.dynamicStateCount = dynamicStateCount;
            pipelineDynamicStateCreateInfo.flags             = flags;
            return pipelineDynamicStateCreateInfo;
        }

        inline VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(
            const std::vector<VkDynamicState>& pDynamicStates,
            VkPipelineDynamicStateCreateFlags  flags = 0) {
            VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};
            pipelineDynamicStateCreateInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            pipelineDynamicStateCreateInfo.pDynamicStates    = pDynamicStates.data();
            pipelineDynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(pDynamicStates.size());
            pipelineDynamicStateCreateInfo.flags             = flags;
            return pipelineDynamicStateCreateInfo;
        }

        inline VkGraphicsPipelineCreateInfo pipelineCreateInfo(
            VkPipelineLayout      layout,
            VkRenderPass          renderPass,
            VkPipelineCreateFlags flags = 0) {
            VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
            pipelineCreateInfo.sType              = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineCreateInfo.layout             = layout;
            pipelineCreateInfo.renderPass         = renderPass;
            pipelineCreateInfo.flags              = flags;
            pipelineCreateInfo.basePipelineIndex  = -1;
            pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
            return pipelineCreateInfo;
        }

        inline VkGraphicsPipelineCreateInfo pipelineCreateInfo() {
            VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
            pipelineCreateInfo.sType              = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineCreateInfo.basePipelineIndex  = -1;
            pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
            return pipelineCreateInfo;
        }

        inline VkVertexInputBindingDescription vertexInputBindingDescription(
            uint32_t          binding,
            uint32_t          stride,
            VkVertexInputRate inputRate) {
            VkVertexInputBindingDescription vInputBindDescription{};
            vInputBindDescription.binding   = binding;
            vInputBindDescription.stride    = stride;
            vInputBindDescription.inputRate = inputRate;
            return vInputBindDescription;
        }

        inline VkImageCreateInfo imageCreateInfo() {
            VkImageCreateInfo imageCreateInfo{};
            imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            return imageCreateInfo;
        }

        inline VkVertexInputAttributeDescription vertexInputAttributeDescription(
            uint32_t binding,
            uint32_t location,
            VkFormat format,
            uint32_t offset) {
            VkVertexInputAttributeDescription vInputAttribDescription{};
            vInputAttribDescription.location = location;
            vInputAttribDescription.binding  = binding;
            vInputAttribDescription.format   = format;
            vInputAttribDescription.offset   = offset;
            return vInputAttribDescription;
        }

        inline VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo() {
            VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
            pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            return pipelineVertexInputStateCreateInfo;
        }

        inline VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo(
            const std::vector<VkVertexInputBindingDescription>&   vertexBindingDescriptions,
            const std::vector<VkVertexInputAttributeDescription>& vertexAttributeDescriptions) {
            VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
            pipelineVertexInputStateCreateInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount   = static_cast<uint32_t>(vertexBindingDescriptions.size());
            pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions      = vertexBindingDescriptions.data();
            pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributeDescriptions.size());
            pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions    = vertexAttributeDescriptions.data();
            return pipelineVertexInputStateCreateInfo;
        }

    }// namespace initializers

    void setImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

    void setImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange);

    void set_image_layout(
        VkCommandBuffer         command_buffer,
        VkImage                 image,
        VkImageLayout           old_layout,
        VkImageLayout           new_layout,
        VkImageSubresourceRange subresource_range,
        VkPipelineStageFlags    src_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VkPipelineStageFlags    dst_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

    VkPipelineRasterizationConservativeStateCreateInfoEXT rasterizationConservativeStateCreateInfo(float extraPrimitiveOverestimationSize, VkConservativeRasterizationModeEXT mode = VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT);

    VkPhysicalDeviceConservativeRasterizationPropertiesEXT conservativeRasterizationProperties(VkPhysicalDevice physicalDevice);
};