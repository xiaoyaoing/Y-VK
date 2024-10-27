#pragma once

#include "Core/Vulkan.h"
#include "RenderGraph/Enum.h"

class ImageUtil {
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

    static std::tuple<VkAccessFlags, VkAccessFlags, VkPipelineStageFlags2, VkPipelineStageFlags2, VkImageLayout, VkImageLayout>
    getVkTransition(VulkanLayout oldLayout, VulkanLayout newLayout);

    static VulkanLayout chooseVulkanLayout(VulkanLayout layout, VulkanLayout defaultLayout);


    inline static VkPipelineStageFlags2 getStageFlags(RenderPassType type) {
        VkPipelineStageFlags2 flags = 0;
        switch(type) {
            case RenderPassType::COMPUTE:
                flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            break;
            case RenderPassType::GRAPHICS:
                flags |= VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
            break;
            case RenderPassType::RAYTRACING:
                flags |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
            break;
        }
        return flags;
    }

    inline static VulkanLayout getDefaultLayout(TextureUsage usage) {
        if (any(usage & TextureUsage::STORAGE))
            return VulkanLayout::READ_WRITE;

        if (any(usage & TextureUsage::SAMPLEABLE))
            return VulkanLayout::READ_ONLY;

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
        if (any(usage & TextureUsage::DEPTH_READ_ONLY))
            return VulkanLayout::DEPTH_READ_ONLY;
        if (any(usage & TextureUsage::TRANSFER_SRC))
            return VulkanLayout::TRANSFER_SRC;
        if (any(usage & TextureUsage::TRANSFER_DST))
            return VulkanLayout::TRANSFER_DST;
        return VulkanLayout::READ_ONLY;
    }

    // inline static VulkanLayout getDefaultLayout(VkImageUsageFlags vkusage)
    // {
    //     TextureUsage usage{};
    //     if (vkusage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
    //     {
    //         usage = usage | TextureUsage::DEPTH_ATTACHMENT;
    //     }
    //     if (vkusage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
    //     {
    //         usage = usage | TextureUsage::COLOR_ATTACHMENT;
    //     }
    //     if (vkusage & VK_IMAGE_USAGE_SAMPLED_BIT)
    //     {
    //         usage = usage | TextureUsage::SAMPLEABLE;
    //     }
    //     return getDefaultLayout(usage);
    // }

    inline static VkImageUsageFlags getUsageFlags(TextureUsage usage) {
        VkImageUsageFlags       vkUsage{};
        const VkImageUsageFlags blittable = VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

        if (any(usage & TextureUsage::SAMPLEABLE)) {
            vkUsage |= VK_IMAGE_USAGE_SAMPLED_BIT;
        }
        if (any(usage & TextureUsage::COLOR_ATTACHMENT)) {
            vkUsage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;//| blittable;
        }
        if (any(usage & TextureUsage::STENCIL_ATTACHMENT)) {
            vkUsage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }
        if (any(usage & TextureUsage::UPLOADABLE)) {
            //vkUsage |= blittable;
        }
        if (any(usage & TextureUsage::DEPTH_ATTACHMENT)) {
            // vkUsage |= blittable;
            vkUsage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

            // Depth resolves uses a custom shader and therefore needs to be sampleable.
            // if (samples > 1) {
            //     vkUsage |= VK_IMAGE_USAGE_SAMPLED_BIT;
            // }
        }
        if (any(usage & TextureUsage::SUBPASS_INPUT)) {
            vkUsage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        }
        if (any(usage & TextureUsage::STORAGE)) {
            vkUsage |= VK_IMAGE_USAGE_STORAGE_BIT;
        }
        if (any(usage & TextureUsage::TRANSFER_SRC))
            vkUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        if (any(usage & TextureUsage::TRANSFER_DST))
            vkUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        return vkUsage;
    }

    inline static VkFormat getFormat(TextureUsage usage) {
        if (any(usage & TextureUsage::DEPTH_ATTACHMENT)) {
            return VK_FORMAT_D32_SFLOAT;
        }
        if (any(usage & TextureUsage::STORAGE))
            return VK_FORMAT_B8G8R8A8_UNORM;
        return VK_FORMAT_R8G8B8A8_SRGB;
    }
};

inline bool isDepthOnlyFormat(VkFormat format) {
    return format == VK_FORMAT_D16_UNORM || format == VK_FORMAT_D32_SFLOAT || format == VK_FORMAT_D16_UNORM_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT_S8_UINT;
}

inline bool isDepthOrStencilFormat(VkFormat format) {
    return isDepthOnlyFormat(format) || format == VK_FORMAT_S8_UINT;
}
