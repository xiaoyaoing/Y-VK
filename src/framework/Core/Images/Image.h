
#pragma once
#include <unordered_map>

#include "Core/Vulkan.h"

class CommandBuffer;
class ImageView;
class Device;

class Image {
public:
    Image(Device&               device,
          const VkExtent3D&     extent,
          VkFormat              format,
          VkImageUsageFlags     image_usage,
          VmaMemoryUsage        memory_usage,
          VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT,
          uint32_t              mip_levels   = 1,
          uint32_t              array_layers = 1,
          VkImageCreateFlags    flags        = 0,
          VkImageType           type         = VK_IMAGE_TYPE_2D);

    Image(Device&               device,
          VkImage               handle,
          const VkExtent3D&     extent,
          VkFormat              format,
          VkImageUsageFlags     image_usage,
          VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT);

    Image& operator=(const Image&) = delete;

    Image& operator=(Image&&) = delete;

    Image(const Image&) = delete;

    ~Image();

    Image(Image&& other);

    VkImageSubresource getSubresource() const;

    inline VkImage getHandle() const {
        return image;
    };

    inline VkFormat getFormat() const {
        return format;
    }

    inline VkImageType getImageType() const {
        return type;
    }

    inline VkSampleCountFlagBits getSampleCount() const {
        return VK_SAMPLE_COUNT_1_BIT;
    };

    inline VkImageUsageFlags getUseFlags() const {
        return usage;
    };

    inline const VkExtent3D getExtent() const {
        return extent;
    }

    uint32_t getArrayLayerCount() const;

    uint32_t getMipLevelCount() const;

    void         transitionLayout(CommandBuffer& commandBuffer, VulkanLayout newLayout, const VkImageSubresourceRange& subresourceRange = {
                                                                                            .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                                                                                            .baseMipLevel   = 0,
                                                                                            .levelCount     = 1,
                                                                                            .baseArrayLayer = 0,
                                                                                            .layerCount     = 1,
                                                                                });
    void         setLayout(VulkanLayout                   newLayout,
                           const VkImageSubresourceRange& subresourceRange = {
                               .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                               .baseMipLevel   = 0,
                               .levelCount     = 1,
                               .baseArrayLayer = 0,
                               .layerCount     = 1,
                   });
    VulkanLayout getLayout(const VkImageSubresourceRange& subresourceRange = {
                               .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                               .baseMipLevel   = 0,
                               .levelCount     = 1,
                               .baseArrayLayer = 0,
                               .layerCount     = 1,
                           });

    Image(VmaAllocator allocator, VmaMemoryUsage memoryUsage, const VkImageCreateInfo& createInfo);

    static inline VkImageCreateInfo getDefaultImageInfo() {
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.pNext             = nullptr;
        imageInfo.mipLevels         = 1;
        imageInfo.arrayLayers       = 1;
        imageInfo.samples           = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode       = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.tiling            = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
        return imageInfo;
    }

    void addView(ImageView* pView);

    Device& getDevice();

    VkImageSubresource subresource{};

protected:
    Device&       device;
    VkImage       image;
    VmaAllocation memory{};

    VkExtent3D extent{};

    VkImageType type{VK_IMAGE_TYPE_MAX_ENUM};

    VkFormat format{};

    VkImageUsageFlags usage{};

    VkSampleCountFlagBits sample_count{};

    VkImageTiling tiling{};

    uint32_t array_layer_count{1};

    uint32_t mip_level_count{1};

    uint8_t* mapped_data{nullptr};

    bool mapped;

    std::unordered_map<std::size_t, VulkanLayout> layouts;
};

//struct HwTexture
//{
//    std::unique_ptr<Image> image;
//    std::unique_ptr<ImageView> view;
//};
