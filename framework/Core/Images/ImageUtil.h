#pragma once

#include "Core/Vulkan.h"

class ImageUtil {
public:
    static VkImageLayout getVkImageLayout(VulkanLayout layout) {
        switch (layout) {
            case VulkanLayout::UNDEFINED:
                return VK_IMAGE_LAYOUT_UNDEFINED;
            case VulkanLayout::READ_WRITE:
                return VK_IMAGE_LAYOUT_GENERAL;
            case VulkanLayout::READ_ONLY:
                return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            case VulkanLayout::TRANSFER_SRC:
                return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            case VulkanLayout::TRANSFER_DST:
                return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            case VulkanLayout::DEPTH_ATTACHMENT:
                return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            case VulkanLayout::DEPTH_SAMPLER:
                return VK_IMAGE_LAYOUT_GENERAL;
            case VulkanLayout::PRESENT:
                return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            case VulkanLayout::COLOR_ATTACHMENT:
                return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            case VulkanLayout::COLOR_ATTACHMENT_RESOLVE:
                return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
    }

    static std::tuple<VkAccessFlags, VkAccessFlags, VkPipelineStageFlags, VkPipelineStageFlags,
            VkImageLayout, VkImageLayout>
    getVkTransition(VulkanLayout oldLayout, VulkanLayout newLayout) {
        VkAccessFlags srcAccessMask, dstAccessMask;
        VkPipelineStageFlags srcStage, dstStage;

        switch (oldLayout) {
            case VulkanLayout::UNDEFINED:
                srcAccessMask = 0;
                srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                break;
            case VulkanLayout::COLOR_ATTACHMENT:
                srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT
                                | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
                                | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                srcStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
                           | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                break;
            case VulkanLayout::READ_WRITE:
                srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                break;
            case VulkanLayout::READ_ONLY:
                srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                break;
            case VulkanLayout::TRANSFER_SRC:
                srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                break;
            case VulkanLayout::TRANSFER_DST:
                srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                break;
            case VulkanLayout::DEPTH_ATTACHMENT:
                srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT
                                | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                srcStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                break;
            case VulkanLayout::DEPTH_SAMPLER:
                srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
                srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                break;
            case VulkanLayout::COLOR_ATTACHMENT_RESOLVE:
                srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                break;
            case VulkanLayout::PRESENT:
                srcAccessMask = VK_ACCESS_NONE;
                srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                break;
        }

        switch (newLayout) {
            case VulkanLayout::COLOR_ATTACHMENT:
                dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT
                                | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
                                | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                dstStage = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
                           | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                break;
            case VulkanLayout::READ_WRITE:
                dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                break;
            case VulkanLayout::READ_ONLY:
                dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                break;
            case VulkanLayout::TRANSFER_SRC:
                dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                break;
            case VulkanLayout::TRANSFER_DST:
                dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                break;
            case VulkanLayout::DEPTH_ATTACHMENT:
                dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT
                                | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
                break;
            case VulkanLayout::DEPTH_SAMPLER:
                dstAccessMask
                        = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
                           | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
                break;
            case VulkanLayout::PRESENT:
            case VulkanLayout::COLOR_ATTACHMENT_RESOLVE:
            case VulkanLayout::UNDEFINED:
                dstAccessMask = 0;
                dstStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                break;
        }

        return std::make_tuple(srcAccessMask, dstAccessMask, srcStage, dstStage,
                               getVkImageLayout(oldLayout), getVkImageLayout(newLayout));
    }


    inline static VulkanLayout getDefaultLayout(TextureUsage usage) {
        if (any(usage & TextureUsage::DEPTH_ATTACHMENT)) {
            if (any(usage & TextureUsage::SAMPLEABLE)) {
                return VulkanLayout::DEPTH_SAMPLER;
            } else {
                return VulkanLayout::DEPTH_ATTACHMENT;
            }
        }

        if (any(usage & TextureUsage::COLOR_ATTACHMENT)) {
            return VulkanLayout::COLOR_ATTACHMENT;
        }
        // Finally, the layout for an immutable texture is optimal read-only.        
        return VulkanLayout::READ_ONLY;
    }

    inline static VulkanLayout getDefaultLayout(VkImageUsageFlags vkusage) {
        TextureUsage usage{};
        if (vkusage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            usage = usage | TextureUsage::DEPTH_ATTACHMENT;
        }
        if (vkusage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
            usage = usage | TextureUsage::COLOR_ATTACHMENT;
        }
        if (vkusage & VK_IMAGE_USAGE_SAMPLED_BIT) {
            usage = usage | TextureUsage::SAMPLEABLE;
        }
        return getDefaultLayout(usage);
    }

    inline static VkImageUsageFlags getUsageFlags(TextureUsage usage) {
        VkImageUsageFlags flags{};
        if (any(usage & TextureUsage::DEPTH_ATTACHMENT)) {
            flags = flags | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }
        if (any(usage & TextureUsage::COLOR_ATTACHMENT)) {
            flags = flags | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            if (any(usage & TextureUsage::SUBPASS_INPUT)) {
                flags = flags | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
            }
        }
        if (any(usage & TextureUsage::SAMPLEABLE)) {
            flags = flags | VK_IMAGE_USAGE_SAMPLED_BIT;
        }

        return flags;
    }

    inline static VkFormat getFormat(TextureUsage usage) {
        if (any(usage & TextureUsage::DEPTH_ATTACHMENT)) {
            return VK_FORMAT_D32_SFLOAT;
        }

        return VK_FORMAT_R8G8B8A8_SRGB;
    }
};


inline bool isDepthOnlyFormat(VkFormat format) {
    return format == VK_FORMAT_D16_UNORM || format == VK_FORMAT_D32_SFLOAT
           || format == VK_FORMAT_D16_UNORM_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT
           || format == VK_FORMAT_D32_SFLOAT_S8_UINT;
}

inline bool isDepthOrStencilFormat(VkFormat format) {
    return isDepthOnlyFormat(format) || format == VK_FORMAT_S8_UINT;
}
