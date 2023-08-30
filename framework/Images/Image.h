#include "Vulkan.h"
#include "ImageView.h"

#pragma once

class Image {

public:
    Image(Device &device,
          const VkExtent3D &extent,
          VkFormat format,
          VkImageUsageFlags image_usage,
          VmaMemoryUsage memory_usage,
          VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT,
          uint32_t mip_levels = 1,
          uint32_t array_layers = 1,
          VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
          VkImageCreateFlags flags = 0,
          uint32_t num_queue_families = 0,
          const uint32_t *queue_families = nullptr);

    Image(Device &device,
          VkImage handle,
          const VkExtent3D &extent,
          VkFormat format,
          VkImageUsageFlags image_usage,
          VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT);


    Image &operator=(const Image &) = delete;

    Image &operator=(Image &&) = delete;

    Image(const Image &) = delete;


    Image(Image &&other);

    inline VkImage getHandle() {
        return image;
    };

    inline VkFormat getFormat() {
        return format;
    }

    inline VkImageType getImageType() {
        return type;
    }

    inline VkSampleCountFlagBits getSampleCount() {
        return VK_SAMPLE_COUNT_1_BIT;
    };

    inline VkImageUsageFlags getUseFlags() {
        return usage;
    };

    inline const VkExtent3D getExtent() {
        return extent;
    }

    uint32_t getArrayLayerCount() const;

    Image(VmaAllocator allocator, VmaMemoryUsage memoryUsage, const VkImageCreateInfo &createInfo);

    static inline VkImageCreateInfo getDefaultImageInfo() {
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.pNext = nullptr;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        return imageInfo;
    }

    void addView(ImageView *pView);

    Device &getDevice();

    VkImageSubresource subresource{};


protected:
    Device &device;
    VkImage image;
    VmaAllocation memory;

    VkExtent3D extent{};

    VkImageType type{};

    VkFormat format{};

    VkImageUsageFlags usage{};

    VkSampleCountFlagBits sample_count{};

    VkImageTiling tiling{};


    uint32_t array_layer_count{1};

    uint8_t *mapped_data{nullptr};

    bool mapped;

    std::vector<ImageView> views;
};