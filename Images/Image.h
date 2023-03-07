#include "Vulkan.h"

class Image {
    VkImage _image;
    VmaAllocator _allocator;
    VmaAllocation _allocation;
    VkExtent3D _dimension;
    VkFormat _imageFormat;
    VkImageType _imageType;
public:
    inline VkImage getHandle() {
        return _image;
    };

    Image(VmaAllocator allocator, VmaMemoryUsage memoryUsage,const VkImageCreateInfo &createInfo);
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
};