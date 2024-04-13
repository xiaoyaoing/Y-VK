//
// Created by pc on 2023/8/17.
//

#include "VkCommon.h"
#include "Core/Texture.h"
#include "Core/Images/ImageView.h"

#define VK_KHR_get_physical_device_properties2 1

namespace vkCommon {

    VkViewport initializers::viewport(float width, float height, float minDepth, float maxDepth) {
            VkViewport viewport{};
            viewport.width    = width;
            viewport.height   = -height;
            viewport.x        = 0;
            viewport.y        = height;
            viewport.minDepth = minDepth;
            viewport.maxDepth = maxDepth;
            return viewport;
        
    }

    void set_image_layout(
        VkCommandBuffer         command_buffer,
        VkImage                 image,
        VkImageLayout           old_layout,
        VkImageLayout           new_layout,
        VkImageSubresourceRange subresource_range,
        VkPipelineStageFlags    src_mask,
        VkPipelineStageFlags    dst_mask) {
        // Create an image barrier object
        VkImageMemoryBarrier barrier{};
        barrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.oldLayout           = old_layout;
        barrier.newLayout           = new_layout;
        barrier.image               = image;
        barrier.subresourceRange    = subresource_range;

        // Source layouts (old)
        // Source access mask controls actions that have to be finished on the old layout
        // before it will be transitioned to the new layout
        switch (old_layout) {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                // Image layout is undefined (or does not matter)
                // Only valid as initial layout
                // No flags required, listed only for completeness
                barrier.srcAccessMask = 0;
                break;

            case VK_IMAGE_LAYOUT_PREINITIALIZED:
                // Image is preinitialized
                // Only valid as initial layout for linear images, preserves memory contents
                // Make sure host writes have been finished
                barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                // Image is a color attachment
                // Make sure any writes to the color buffer have been finished
                barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                // Image is a depth/stencil attachment
                // Make sure any writes to the depth/stencil buffer have been finished
                barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                // Image is a transfer source
                // Make sure any reads from the image have been finished
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                // Image is a transfer destination
                // Make sure any writes to the image have been finished
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                // Image is read by a shader
                // Make sure any shader reads from the image have been finished
                barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                break;
            default:
                // Other source layouts aren't handled (yet)
                break;
        }

        // Target layouts (new)
        // Destination access mask controls the dependency for the new image layout
        switch (new_layout) {
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                // Image will be used as a transfer destination
                // Make sure any writes to the image have been finished
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                // Image will be used as a transfer source
                // Make sure any reads from the image have been finished
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;

            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                // Image will be used as a color attachment
                // Make sure any writes to the color buffer have been finished
                barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                // Image layout will be used as a depth/stencil attachment
                // Make sure any writes to depth/stencil buffer have been finished
                barrier.dstAccessMask = barrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;

            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                // Image will be read in a shader (sampler, input attachment)
                // Make sure any writes to the image have been finished
                if (barrier.srcAccessMask == 0) {
                    barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
                }
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                break;
            default:
                // Other source layouts aren't handled (yet)
                break;
        }

        // Put barrier inside setup command buffer
        vkCmdPipelineBarrier(
            command_buffer,
            src_mask,
            dst_mask,
            0,
            0,
            nullptr,
            0,
            nullptr,
            1,
            &barrier);
    }

    VkPipelineRasterizationConservativeStateCreateInfoEXT rasterizationConservativeStateCreateInfo(float extraPrimitiveOverestimationSize, VkConservativeRasterizationModeEXT mode) {
        VkPipelineRasterizationConservativeStateCreateInfoEXT rasterization_conservative_state_create_info{};
        rasterization_conservative_state_create_info.sType                            = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT;
        rasterization_conservative_state_create_info.flags                            = 0;
        rasterization_conservative_state_create_info.pNext                            = nullptr;
        rasterization_conservative_state_create_info.conservativeRasterizationMode    = mode;
        rasterization_conservative_state_create_info.extraPrimitiveOverestimationSize = extraPrimitiveOverestimationSize;
        return rasterization_conservative_state_create_info;
    }

    VkPhysicalDeviceConservativeRasterizationPropertiesEXT conservativeRasterizationProperties(VkPhysicalDevice physicalDevice) {
        // PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2KHR>(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties2KHR"));
        //    assert(vkGetPhysicalDeviceProperties2KHR);

        VkPhysicalDeviceConservativeRasterizationPropertiesEXT conservativeRasterProps{};
        VkPhysicalDeviceProperties2KHR                         deviceProps2{};
        conservativeRasterProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT;
        deviceProps2.sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
        deviceProps2.pNext            = &conservativeRasterProps;
        vkGetPhysicalDeviceProperties2KHR(physicalDevice, &deviceProps2);
        return conservativeRasterProps;
    }
}