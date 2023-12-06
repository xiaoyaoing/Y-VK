#include "ImageUtil.h"

VkImageLayout ImageUtil::getVkImageLayout(VulkanLayout layout)
{
    {
        switch (layout)
        {
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
          //  return VK_IMAGE_LAYOUT_GENERAL;
        case VulkanLayout::DEPTH_SAMPLER:
            return VK_IMAGE_LAYOUT_GENERAL;
        case VulkanLayout::DEPTH_READ_ONLY:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        case VulkanLayout::PRESENT:
            return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        case VulkanLayout::COLOR_ATTACHMENT:
          //  return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            return VK_IMAGE_LAYOUT_GENERAL;
        case VulkanLayout::COLOR_ATTACHMENT_RESOLVE:
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
    }
}

VulkanLayout ImageUtil::chooseVulkanLayout(VulkanLayout layout, VulkanLayout defaultLayout)
{
    return layout == VulkanLayout::UNDEFINED ? defaultLayout : layout;
}

std::tuple<VkAccessFlags, VkAccessFlags, VkPipelineStageFlags, VkPipelineStageFlags, VkImageLayout, VkImageLayout>
ImageUtil::getVkTransition(VulkanLayout oldLayout, VulkanLayout newLayout)
{
        VkAccessFlags srcAccessMask, dstAccessMask;
        VkPipelineStageFlags srcStage, dstStage;

        switch (oldLayout)
        {
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
        case VulkanLayout::DEPTH_READ_ONLY:
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

        switch (newLayout)
        {
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
        case VulkanLayout::DEPTH_READ_ONLY:
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


