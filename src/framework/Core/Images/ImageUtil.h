#pragma once

#include "Core/Vulkan.h"

class ImageUtil
{
public:
    static VkImageLayout getVkImageLayout(VulkanLayout layout);
    // static VulkanLayout ImageUtil::getVulkanLayout(VkImageLayout layout)
    // {
    //     switch (layout)
    //     {
    //     case VK_IMAGE_LAYOUT_UNDEFINED:
    //         return VulkanLayout::UNDEFINED;
    //     case VK_IMAGE_LAYOUT_GENERAL:
    //         return VulkanLayout::READ_WRITE;
    //     case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
    //         return VulkanLayout::READ_ONLY;
    //     case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
    //         return VulkanLayout::TRANSFER_SRC;
    //     case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
    //         return VulkanLayout::TRANSFER_DST;
    //     case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
    //         return VulkanLayout::DEPTH_ATTACHMENT;
    //     case VK_IMAGE_LAYOUT_GENERAL:
    //         return VulkanLayout::DEPTH_SAMPLER;
    //     case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
    //         return VulkanLayout::DEPTH_READ_ONLY;
    //     case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
    //         return VulkanLayout::PRESENT;
    //     case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
    //         return VulkanLayout::COLOR_ATTACHMENT;
    //     }
    //
    //     // 默认情况，可以根据需要返回特定的默认值或抛出异常
    //     return VulkanLayout::UNDEFINED;
    // }


    static std::tuple<VkAccessFlags, VkAccessFlags, VkPipelineStageFlags, VkPipelineStageFlags,
                      VkImageLayout, VkImageLayout>
    getVkTransition(VulkanLayout oldLayout, VulkanLayout newLayout);

    static VulkanLayout chooseVulkanLayout(VulkanLayout layout,VulkanLayout defaultLayout);

    inline static VulkanLayout getDefaultLayout(TextureUsage usage)
    {
        if (any(usage & TextureUsage::DEPTH_ATTACHMENT))
        {
            if (any(usage & TextureUsage::SAMPLEABLE))
            {
                return VulkanLayout::DEPTH_SAMPLER;
            }
            else
            {
                return VulkanLayout::DEPTH_ATTACHMENT;
            }
        }

        if (any(usage & TextureUsage::COLOR_ATTACHMENT))
        {
            return VulkanLayout::COLOR_ATTACHMENT;
        }
        // Finally, the layout for an immutable texture is optimal read-only.
        if(any(usage & TextureUsage::DEPTH_READ_ONLY))
            return VulkanLayout::DEPTH_READ_ONLY;
        return VulkanLayout::READ_ONLY;
    }

    inline static VulkanLayout getDefaultLayout(VkImageUsageFlags vkusage)
    {
        TextureUsage usage{};
        if (vkusage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            usage = usage | TextureUsage::DEPTH_ATTACHMENT;
        }
        if (vkusage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
        {
            usage = usage | TextureUsage::COLOR_ATTACHMENT;
        }
        if (vkusage & VK_IMAGE_USAGE_SAMPLED_BIT)
        {
            usage = usage | TextureUsage::SAMPLEABLE;
        }
        return getDefaultLayout(usage);
    }

    inline static VkImageUsageFlags getUsageFlags(TextureUsage usage)
    {
        VkImageUsageFlags flags{};
        if (any(usage & TextureUsage::DEPTH_ATTACHMENT))
        {
            flags = flags | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }
        if (any(usage & TextureUsage::COLOR_ATTACHMENT))
        {
            flags = flags | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
        if (any(usage & TextureUsage::SUBPASS_INPUT))
        {
            flags = flags | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        }
        if (any(usage & TextureUsage::SAMPLEABLE))
        {
            flags = flags | VK_IMAGE_USAGE_SAMPLED_BIT;
        }

        return flags;
    }

    inline static VkFormat getFormat(TextureUsage usage)
    {
        if (any(usage & TextureUsage::DEPTH_ATTACHMENT))
        {
            return VK_FORMAT_D32_SFLOAT;
        }

        return VK_FORMAT_R8G8B8A8_SRGB;
    }
};


inline bool isDepthOnlyFormat(VkFormat format)
{
    return format == VK_FORMAT_D16_UNORM || format == VK_FORMAT_D32_SFLOAT
        || format == VK_FORMAT_D16_UNORM_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT
        || format == VK_FORMAT_D32_SFLOAT_S8_UINT;
}

inline bool isDepthOrStencilFormat(VkFormat format)
{
    return isDepthOnlyFormat(format) || format == VK_FORMAT_S8_UINT;
}
