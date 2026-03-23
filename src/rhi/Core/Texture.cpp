//
// Created by pc on 2023/8/18.
//

#include "Texture.h"

#include "Core/Buffer.h"
#include "Core/Device/Device.h"
#include "Common/VkCommon.h"
#include <Core/Images/Sampler.h>

#include "RenderContext.h"
#include "Common/ResourceCache.h"

const SgImage& Texture::getImage() const {
    return *image;
}

const Sampler& Texture::getSampler() const {
    return *sampler;
}

// static void initVKTexture(Device& device, std::unique_ptr<Texture>& texture) {
//     texture->image->generateMipMapOnCpu();
//     texture->image->createVkImage(device);
//
//     auto imageBuffer = Buffer(device, texture->image->getBufferSize(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
//     imageBuffer.uploadData(static_cast<void*>(texture->image->getData().data()), texture->image->getBufferSize());
//
//     std::vector<VkBufferImageCopy> imageCopyRegions;
//
//     uint32_t levelCount = texture->image->getLevels();
//     auto&    mipmaps    = texture->image->getMipMaps();
//     for (int i = 0; i < mipmaps.size(); i++) {
//         VkBufferImageCopy imageCopy{};
//         imageCopy.bufferRowLength                 = 0;
//         imageCopy.bufferImageHeight               = 0;
//         imageCopy.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
//         imageCopy.bufferOffset                    = mipmaps[i].offset;
//         imageCopy.imageSubresource.mipLevel       = toUint32(i % levelCount);
//         imageCopy.imageSubresource.baseArrayLayer = i / levelCount;
//         imageCopy.imageSubresource.layerCount     = 1;
//         imageCopy.imageOffset                     = {0, 0, 0};
//         imageCopy.imageExtent                     = texture->image->getMipMaps()[i].extent;
//         imageCopyRegions.push_back(imageCopy);
//     }
//
//     auto                    commandBuffer = device.createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
//     VkImageSubresourceRange subresourceRange{};
//     subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
//     subresourceRange.baseMipLevel   = 0;
//     subresourceRange.baseArrayLayer = 0;
//     subresourceRange.levelCount     = mipmaps.size();
//     subresourceRange.layerCount     = texture->image->getLayers();
//
//     texture->getImage().getVkImage().transitionLayout(commandBuffer, VulkanLayout::TRANSFER_DST, subresourceRange);
//
//     commandBuffer.copyBufferToImage(imageBuffer, texture->image->getVkImage(), imageCopyRegions);
//
//     texture->getImage().getVkImage().transitionLayout(commandBuffer, VulkanLayout::READ_ONLY, subresourceRange);
//
//     g_context->submit(commandBuffer,true,VK_QUEUE_TRANSFER_BIT);
//     texture->sampler = std::make_unique<Sampler>(device, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_FILTER_LINEAR, mipmaps.size());
// }
static void initVKTexture(Device&                   device,
                          std::unique_ptr<Texture>& texture,
                          CommandBuffer&            commandBuffer,
                          Buffer&                   imageBuffer,
                          uint32_t                  offset) {
    texture->image->createVkImage(device);
    imageBuffer.uploadData(static_cast<void*>(texture->image->getData().data()), texture->image->getBufferSize(), offset);

    std::vector<VkBufferImageCopy> imageCopyRegions;

    uint32_t layerCount = texture->image->getArrayLayerCount();
    auto&    mipmaps    = texture->image->getMipMaps();

    for (int i = 0; i < mipmaps.size(); i++) {
        VkBufferImageCopy imageCopy{};
        imageCopy.bufferRowLength                 = 0;
        imageCopy.bufferImageHeight               = 0;
        imageCopy.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopy.bufferOffset                    = offset + mipmaps[i].offset;
        imageCopy.imageSubresource.mipLevel       = toUint32(i / layerCount);
        imageCopy.imageSubresource.baseArrayLayer = i % layerCount;
        imageCopy.imageSubresource.layerCount     = 1;
        imageCopy.imageOffset                     = {0, 0, 0};
        imageCopy.imageExtent                     = texture->image->getMipMaps()[i].extent;
        imageCopyRegions.push_back(imageCopy);
    }

    VkImageSubresourceRange subresourceRange{};
    subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel   = 0;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.levelCount     = texture->image->getMipMaps().size() / texture->image->getArrayLayerCount();
    subresourceRange.layerCount     = layerCount;

    texture->getImage().getVkImage().transitionLayout(commandBuffer, VulkanLayout::TRANSFER_DST, subresourceRange);

    commandBuffer.copyBufferToImage(imageBuffer, texture->image->getVkImage(), imageCopyRegions);

    //  texture->getImage().getVkImage().transitionLayout(commandBuffer, VulkanLayout::READ_ONLY, subresourceRange);

    if (texture->image->getMipMaps().size() == 1 && texture->image->needGenerateMipMapOnGpu()) {
        std::vector<VkImageBlit2> blits;
        int                       mipWidth  = texture->image->getExtent().width;
        int                       mipHeight = texture->image->getExtent().height;

        int i = 0;
        while (true) {
            VkImageBlit2 blit{};
            blit.sType                         = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;
            blit.srcOffsets[0]                 = {0, 0, 0};
            blit.srcOffsets[1]                 = {mipWidth, mipHeight, 1};
            blit.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel       = i;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount     = 1;
            blit.dstOffsets[0]                 = {0, 0, 0};
            blit.dstOffsets[1]                 = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
            blit.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel       = i + 1;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount     = 1;
            blits                              = {blit};

            mipWidth  = std::max(1, mipWidth / 2);
            mipHeight = std::max(1, mipHeight / 2);
            if (mipWidth <= 1 && mipHeight <= 1)
                break;

            VkBlitImageInfo2 blitInfo{};
            blitInfo.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
            texture->image->getVkImage().transitionLayout(commandBuffer, VulkanLayout::TRANSFER_SRC, {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = static_cast<uint32_t>(i), .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1});
            texture->image->getVkImage().transitionLayout(commandBuffer, VulkanLayout::TRANSFER_DST, {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = static_cast<uint32_t>(i + 1), .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1});
            blitInfo.srcImage       = texture->image->getVkImage().getHandle();
            blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            blitInfo.dstImage       = texture->image->getVkImage().getHandle();
            blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            blitInfo.regionCount    = blits.size();
            blitInfo.pRegions       = blits.data();
            vkCmdBlitImage2(commandBuffer.getHandle(), &blitInfo);//

            i++;
        }
    }

    subresourceRange.levelCount = texture->image->getMipLevelCount();
    texture->getImage().getVkImage().transitionLayout(commandBuffer, VulkanLayout::READ_ONLY, subresourceRange);

    VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode addressModeV = texture->image->getFormat() == VK_FORMAT_R32G32B32A32_SFLOAT ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE : VK_SAMPLER_ADDRESS_MODE_REPEAT;
    texture->sampler                  = &device.getResourceCache().requestSampler(VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_FILTER_LINEAR, texture->image->getMipLevelCount(), addressModeU, addressModeV);
}

std::unique_ptr<Texture> Texture::loadTextureFromFile(Device& device, const std::string& path) {
    std::unique_ptr<Texture> texture = std::make_unique<Texture>();
    texture->image                   = std::make_unique<SgImage>(device, path);
    CommandBuffer commandBuffer      = device.createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    auto          buffer             = Buffer(device, texture->image->getBufferSize(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    initVKTexture(device, texture, commandBuffer, buffer, 0);
    g_context->submit(commandBuffer, true, VK_QUEUE_TRANSFER_BIT);
    return texture;
}
std::unique_ptr<Texture> Texture::loadTextureFromFileWitoutInit(Device& device, const std::string& path) {
    std::unique_ptr<Texture> texture = std::make_unique<Texture>();
    texture->image                   = std::make_unique<SgImage>(device, path);
    if (texture->image->getData().empty())
        return nullptr;
    return texture;
}
std::unique_ptr<Texture> Texture::loadTextureFromMemory(Device& device, std::vector<uint8_t>& data, VkExtent3D extent, VkImageViewType viewType, VkFormat format) {
    std::unique_ptr<Texture> texture = std::make_unique<Texture>();
    texture->image                   = std::make_unique<SgImage>(device, std::move(data), extent, viewType, format);
    CommandBuffer commandBuffer      = device.createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    auto          buffer             = Buffer(device, texture->image->getBufferSize(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    initVKTexture(device, texture, commandBuffer, buffer, 0);
    g_context->submit(commandBuffer, true, VK_QUEUE_TRANSFER_BIT);
    return texture;
}

std::unique_ptr<Texture> Texture::loadTextureFromMemoryWithoutInit(Device& device, std::vector<uint8_t>& data, VkExtent3D extent, VkImageViewType viewType, VkFormat format) {
    std::unique_ptr<Texture> texture = std::make_unique<Texture>();
    texture->image                   = std::make_unique<SgImage>(device, std::move(data), extent, viewType, format);
    return texture;
}

// void GenerateTextureMipmaps(Device& device, std::vector<std::unique_ptr<Texture>> & textures) {
//     // CommandBuffer                        commandBuffer = device.createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
//     // std::vector<std::unique_ptr<Buffer>> buffers;
//     // initVKTexture(device, texture, commandBuffer, buffers);
//     // g_context->submit(commandBuffer,true,VK_QUEUE_TRANSFER_BIT);
//     CommandBuffer                       commandBuffer = device.createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
//     for(int i= 0 ;i< textures.size(); i++){
//         std::vector<std::unique_ptr<Buffer>> buffers;
//         initVKTexture(device, textures[i], commandBuffer, buffers);
//     }
// }

std::unique_ptr<Texture> Texture::loadTextureArrayFromFile(Device& device, const std::string& path) {
    std::unique_ptr<Texture> texture{};

    auto imageBuffer = Buffer(device, texture->image->getBufferSize(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    imageBuffer.uploadData(static_cast<void*>(texture->image->getData().data()), texture->image->getBufferSize());

    std::vector<VkBufferImageCopy> imageCopyRegions;

    auto mipmaps    = texture->image->getMipMaps();
    auto levelCount = texture->image->getMipLevelCount();
    auto layerCount = texture->image->getArrayLayerCount();
    auto offsets    = texture->image->getOffsets();
    //   for (int32_t layer = 0; layer < layerCount; layer++)
    for (int i = 0; i < mipmaps.size(); i++) {
        VkBufferImageCopy imageCopy{};

        imageCopy.bufferRowLength   = 0;
        imageCopy.bufferImageHeight = 0;

        imageCopy.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopy.imageSubresource.mipLevel       = i % levelCount;
        imageCopy.imageSubresource.baseArrayLayer = i / levelCount;
        imageCopy.imageSubresource.layerCount     = 1;

        imageCopy.imageOffset  = {0, 0, 0};
        imageCopy.bufferOffset = mipmaps[i].offset;

        imageCopy.imageExtent.width =
            imageCopy.imageExtent.height = texture->image->getExtent().height >> i;
        imageCopy.imageExtent.depth      = 1;

        imageCopyRegions.push_back(imageCopy);
    }

    auto                    commandBuffer = device.createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    VkImageSubresourceRange subresourceRange{};
    subresourceRange.aspectMask   = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount   = levelCount;
    subresourceRange.layerCount   = layerCount;

    texture->getImage().getVkImage().transitionLayout(commandBuffer, VulkanLayout::TRANSFER_DST, subresourceRange);

    commandBuffer.copyBufferToImage(imageBuffer, texture->image->getVkImage(), imageCopyRegions);

    texture->getImage().getVkImage().transitionLayout(commandBuffer, VulkanLayout::READ_ONLY, subresourceRange);

    commandBuffer.endRecord();

    auto queue = device.getQueueByFlag(VK_QUEUE_GRAPHICS_BIT, 0);

    VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;

    auto vkCmdBuffer           = commandBuffer.getHandle();
    submitInfo.pCommandBuffers = &vkCmdBuffer;

    queue.submit({submitInfo}, VK_NULL_HANDLE);
    texture->sampler = &device.getResourceCache().requestSampler(VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_FILTER_LINEAR, mipmaps.size());
    queue.wait();
    return texture;
}

void initTexturesInBatch(std::vector<std::unique_ptr<Texture>>& textures, int start, int end) {
    if (end <= start) {
        return;
    }
    // start = 31;end =32;
    CommandBuffer commandBuffer = g_context->getDevice().createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true, VK_QUEUE_TRANSFER_BIT);
    uint32_t      size{0}, offset{0};
    for (int i = start; i < end; i++) {
        size += textures[i]->image->getBufferSize();
    }
    auto buffer = Buffer(g_context->getDevice(), size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    for (int i = start; i < end; i++) {
        initVKTexture(g_context->getDevice(), textures[i], commandBuffer, buffer, offset);
        offset += textures[i]->image->getBufferSize();
    }
    g_context->submit(commandBuffer, true, VK_QUEUE_TRANSFER_BIT);
    for (int i = start; i < end; i++) {
        //For hdr image,cpu date may be used in ray tracing scene,for accel construction
        if (textures[i]->getImage().getFormat() != VK_FORMAT_R32G32B32A32_SFLOAT) {
            textures[i]->image->freeImageCpuData();
        }
    }
}

void Texture::initTexturesInOneSubmit(std::vector<std::unique_ptr<Texture>>& textures) {
    if (textures.empty()) {
        return;
    }

    int batchSize = 10;
    for (int i = 0; i < textures.size(); i += batchSize) {
        initTexturesInBatch(textures, i, std::min(i + batchSize, static_cast<int>(textures.size())));
    }

    return;

    CommandBuffer commandBuffer = g_context->getDevice().createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    uint32_t      size{0}, offset{0};
    for (auto& texture : textures) {
        size += texture->image->getBufferSize();
    }
    auto buffer = Buffer(g_context->getDevice(), size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    for (auto& texture : textures) {
        initVKTexture(g_context->getDevice(), texture, commandBuffer, buffer, offset);
        offset += texture->image->getBufferSize();
    }
    g_context->submit(commandBuffer, true, VK_QUEUE_TRANSFER_BIT);
    for (auto& texture : textures) {
        //For hdr image,cpu date may be used in ray tracing scene,for accel construction
        if (texture->getImage().getFormat() != VK_FORMAT_R32G32B32A32_SFLOAT) {
            texture->image->freeImageCpuData();
        }
    }
}

Texture& Texture::operator=(Texture&& rhs) {
    this->image   = std::move(rhs.image);
    this->name    = rhs.name;
    this->sampler = std::move(rhs.sampler);
    return *this;
}
