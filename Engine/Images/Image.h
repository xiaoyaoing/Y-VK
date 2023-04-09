#include "Engine/Vulkan.h"
#include "ImageView.h"

class Image {
    VkImage _image;
    VmaAllocator _allocator;
    VmaAllocation _allocation;
    VkExtent3D _extent;
    VkFormat _format;
    VkImageType _imageType;
    VkImageUsageFlags  _useFlags;
public:
    inline VkImage getHandle() {
        return _image;
    };
    inline  VkFormat getFormat(){
        return _format;
    }
    inline  VkImageType getImageType(){
        return _imageType;
    }
    inline  VkSampleCountFlagBits getSampleCount(){
        return VK_SAMPLE_COUNT_1_BIT;
    };
    inline  VkImageUsageFlags  getUseFlags(){
        return _useFlags;
    };
    inline  const VkExtent3D & getExtent(){
        return _extent;
    }
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

    void addView(ImageView *pView);
};