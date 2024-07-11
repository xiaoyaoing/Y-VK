#include "TextureHelper.h"

#include "Core/RenderContext.h"
#include "Scene/SgImage.h"

extern float samplerBlueNoiseErrorDistribution_128x128_OptimizedFor_2d2d2d2d_1spp(int pixel_i, int pixel_j, int sampleIndex, int sampleDimension);

namespace TextureHelper {

    enum HELPERTEXTURE_COUNT {
        HELPERTEXTURE_LOGO,
        HELPERTEXTURE_BLUENOISE,
        HELPERTEXTURE_COUNT
    };

    static  std::unique_ptr<SgImage> helperTextures[HELPERTEXTURE_COUNT];
    static  std::unique_ptr<Buffer> helperBuffers[HELPERTEXTURE_COUNT];
    static  std::unique_ptr<Sampler> sampler;

    void Initialize() {

        //init blud noise
        {
            std::vector<uint8_t> data;
            std::vector<float> dataFloat;
            data.resize(128 * 128 * 4);
            dataFloat.resize(128 * 128 * 4);
            for (int y = 0; y < 128; ++y) {
                for (int x = 0; x < 128; ++x) {
                    const float f0 = samplerBlueNoiseErrorDistribution_128x128_OptimizedFor_2d2d2d2d_1spp(x, y, 0, 0);
                    const float f1 = samplerBlueNoiseErrorDistribution_128x128_OptimizedFor_2d2d2d2d_1spp(x, y, 0, 1);
                    const float f2 = samplerBlueNoiseErrorDistribution_128x128_OptimizedFor_2d2d2d2d_1spp(x, y, 0, 2);
                    const float f3 = samplerBlueNoiseErrorDistribution_128x128_OptimizedFor_2d2d2d2d_1spp(x, y, 0, 3);

                    data[(x + y * 128) * 4 + 0] = uint8_t(f0 * 255);
                    data[(x + y * 128) * 4 + 1] = uint8_t(f1 * 255);
                    data[(x + y * 128) * 4 + 2] = uint8_t(f2 * 255);
                    data[(x + y * 128) * 4 + 3] = uint8_t(f3 * 255);

                    dataFloat[(x + y * 128) * 4 + 0] = f0;
                    dataFloat[(x + y * 128) * 4 + 1] = f1;
                    dataFloat[(x + y * 128) * 4 + 2] = f2;
                    dataFloat[(x + y * 128) * 4 + 3] = f3;
                }
            }
            VkExtent3D               extent    = {128, 128, 1};
            std::unique_ptr<SgImage> blueNoise = std::make_unique<SgImage>(g_context->getDevice(), data, extent, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM);
            blueNoise->createVkImage(g_context->getDevice());
            auto              commandBuffer = g_context->getDevice().createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true, VK_QUEUE_TRANSFER_BIT);
            Buffer            imageBuffer   = Buffer(g_context->getDevice(), data.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, data.data());
            VkBufferImageCopy imageCopy{};
            imageCopy.bufferRowLength                 = 0;
            imageCopy.bufferImageHeight               = 0;
            imageCopy.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            imageCopy.bufferOffset                    = 0;
            imageCopy.imageSubresource.mipLevel       = 0;
            imageCopy.imageSubresource.baseArrayLayer = 0;
            imageCopy.imageSubresource.layerCount     = 1;
            imageCopy.imageOffset                     = {0, 0, 0};
            imageCopy.imageExtent                     = {128, 128};

            VkImageSubresourceRange subresourceRange{};
            subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            subresourceRange.baseMipLevel   = 0;
            subresourceRange.baseArrayLayer = 0;
            subresourceRange.levelCount     = blueNoise->getMipLevelCount();
            subresourceRange.layerCount     = 1;

            blueNoise->getVkImage().transitionLayout(commandBuffer, VulkanLayout::TRANSFER_DST, subresourceRange);

            commandBuffer.copyBufferToImage(imageBuffer, blueNoise->getVkImage(), {imageCopy});

            blueNoise->getVkImage().transitionLayout(commandBuffer, VulkanLayout::READ_ONLY, subresourceRange);
            g_context->submit(commandBuffer);
            helperTextures[HELPERTEXTURE_BLUENOISE] = std::move(blueNoise);

            helperBuffers[HELPERTEXTURE_BLUENOISE] = std::make_unique<Buffer>(g_context->getDevice(), data.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, dataFloat.data());
            // helperBuffers[HELPERTEXTURE_BLUENOISE]->uploadData(data.data(),data.size());
            // auto addr = helperBuffers[HELPERTEXTURE_BLUENOISE]->map();
            // LOGI("BlueNoise addr {}", uint64_t(addr));
            // memcpy(data.data(),addr, data.size());
            // helperBuffers[HELPERTEXTURE_BLUENOISE]->unmap();
            
        }

        {
        }
    }
    const SgImage* GetLogo() {
        return helperTextures[HELPERTEXTURE_LOGO].get();
    }
    const SgImage* GetBlueNoise() {
        return helperTextures[HELPERTEXTURE_BLUENOISE].get();
    }

     Buffer* GetBlueNoiseBuffer() {
        return helperBuffers[HELPERTEXTURE_BLUENOISE].get();
    }
}