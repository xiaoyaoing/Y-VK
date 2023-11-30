//
// Created by pc on 2023/8/18.
//

#include "Texture.h"

#include "Core/Buffer.h"
#include "Core/Device/Device.h"
#include "Common/VkCommon.h"
#include <Core/Images/Sampler.h>

const SgImage& Texture::getImage() const
{
    return *image;
}

const Sampler& Texture::getSampler() const
{
    return *sampler;
}


Texture Texture::loadTexture(Device& device, const std::string& path)
{
    Texture texture{};
    texture.image = std::make_unique<SgImage>(device, path, VK_IMAGE_VIEW_TYPE_2D);
    //  texture.image->createVkImage(device);

    auto imageBuffer = Buffer(device, texture.image->getBufferSize(),
                              VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              VMA_MEMORY_USAGE_CPU_ONLY);
    imageBuffer.uploadData(static_cast<void*>(texture.image->getData().data()), texture.image->getBufferSize());

    std::vector<VkBufferImageCopy> imageCopyRegions;

    auto& mipmaps = texture.image->getMipMaps();
    for (int i = 0; i < mipmaps.size(); i++)
    {
        VkBufferImageCopy imageCopy{};
        imageCopy.bufferRowLength = 0;
        imageCopy.bufferImageHeight = 0;
        imageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopy.bufferOffset = mipmaps[i].offset;
        imageCopy.imageSubresource.mipLevel = toUint32(i);
        imageCopy.imageSubresource.baseArrayLayer = 0;
        imageCopy.imageSubresource.layerCount = 1;
        imageCopy.imageOffset = {0, 0, 0};
        imageCopy.imageExtent.width = texture.image->getExtent().width >> i;
        imageCopy.imageExtent.height = texture.image->getExtent().height >> i;
        imageCopy.imageExtent.depth = 1;
        imageCopyRegions.push_back(imageCopy);
    }


    auto commandBuffer = device.createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    VkImageSubresourceRange subresourceRange{};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = mipmaps.size();
    subresourceRange.layerCount = 1;


    texture.getImage().getVkImage().transitionLayout(commandBuffer, VulkanLayout::TRANSFER_DST, subresourceRange);

    commandBuffer.copyBufferToImage(imageBuffer, texture.image->getVkImage(), imageCopyRegions);

    texture.getImage().getVkImage().transitionLayout(commandBuffer, VulkanLayout::READ_ONLY, subresourceRange);


    commandBuffer.endRecord();

    auto queue = device.getQueueByFlag(VK_QUEUE_GRAPHICS_BIT, 0);

    VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;

    auto vkCmdBuffer = commandBuffer.getHandle();

    submitInfo.pCommandBuffers = &vkCmdBuffer;
    queue.submit({submitInfo}, VK_NULL_HANDLE);
    texture.sampler = std::make_unique<Sampler>(device, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_FILTER_LINEAR, 1);
    queue.wait();
    return texture;
}

Texture Texture::loadTextureArray(Device& device, const std::string& path)
{
    Texture texture{};

    auto imageBuffer = Buffer(device, texture.image->getBufferSize(),
                              VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              VMA_MEMORY_USAGE_CPU_ONLY);
    imageBuffer.uploadData(static_cast<void*>(texture.image->getData().data()), texture.image->getBufferSize());

    std::vector<VkBufferImageCopy> imageCopyRegions;

    auto mipmaps = texture.image->getMipMaps();
    auto layers = texture.image->getLayers();
    auto offsets = texture.image->getOffsets();
    for (int32_t layer = 0; layer < layers; layer++)
        for (int i = 0; i < mipmaps.size(); i++)
        {
            VkBufferImageCopy imageCopy{};

            imageCopy.bufferRowLength = 0;
            imageCopy.bufferImageHeight = 0;

            imageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageCopy.imageSubresource.mipLevel = toUint32(i);
            imageCopy.imageSubresource.baseArrayLayer = layer;
            imageCopy.imageSubresource.layerCount = 1;

            imageCopy.imageOffset = {0, 0, 0};
            imageCopy.bufferOffset = offsets[layer][i];

            imageCopy.imageExtent.width = texture.image->getExtent().width >> i;
            imageCopy.imageExtent.height = texture.image->getExtent().height >> i;
            imageCopy.imageExtent.depth = 1;

            imageCopyRegions.push_back(imageCopy);
        }


    auto commandBuffer = device.createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    VkImageSubresourceRange subresourceRange{};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = mipmaps.size();
    subresourceRange.layerCount = layers;

    texture.getImage().getVkImage().transitionLayout(commandBuffer, VulkanLayout::TRANSFER_DST, subresourceRange);

    commandBuffer.copyBufferToImage(imageBuffer, texture.image->getVkImage(), imageCopyRegions);

    texture.getImage().getVkImage().transitionLayout(commandBuffer, VulkanLayout::READ_ONLY, subresourceRange);


    commandBuffer.endRecord();

    auto queue = device.getQueueByFlag(VK_QUEUE_GRAPHICS_BIT, 0);

    VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;

    auto vkCmdBuffer = commandBuffer.getHandle();
    submitInfo.pCommandBuffers = &vkCmdBuffer;

    queue.submit({submitInfo}, VK_NULL_HANDLE);
    texture.sampler = std::make_unique<Sampler>(device, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_FILTER_LINEAR,
                                                mipmaps.size());
    queue.wait();
    return texture;
}
