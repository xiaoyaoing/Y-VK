#include "Image.h"

Image::Image(VmaAllocator allocator,VmaMemoryUsage memoryUsage, const VkImageCreateInfo &createInfo) {
    _allocator	 = allocator;
    _imageFormat = createInfo.format;
    _dimension	 = createInfo.extent;
    _imageType	 = createInfo.imageType;

    VmaAllocationCreateInfo allocationInfo = {};
    allocationInfo.usage		 = memoryUsage;
    allocationInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    ASSERT(vmaCreateImage(_allocator,
                       &createInfo, &allocationInfo,
                       &_image, &_allocation, nullptr) == VK_SUCCESS,"Create image");
}
