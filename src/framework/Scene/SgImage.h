//
// Created by pc on 2023/8/18.
//
#pragma once

#include "Core/Vulkan.h"

#include "Core/Images/ImageView.h"

#include "Core/Images/Image.h"


struct Mipmap {
    /// Mipmap level
    uint32_t level = 0;

    /// Byte offset used for uploading
    uint32_t offset = 0;

    /// Width depth and height of the mipmap
    VkExtent3D extent = {0, 0, 0};
};

class SgImage {
public:
    
    /**
     * \brief load from hardware texture
     */
    SgImage(Device &device, const std::string &path, VkImageViewType viewType);

    /**
     * \brief create from image attribute 
     */
    SgImage(Device &device,
           const std::string &name,
           const VkExtent3D &extent,
           VkFormat format,
           VkImageUsageFlags image_usage,
           VmaMemoryUsage memory_usage,
           VkImageViewType viewType,
           VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT,
           uint32_t mip_levels = 1,
           uint32_t array_layers = 1,
           VkImageCreateFlags flags = 0);

    /**
     * \brief load from existing image.Mainly from swapChainImage
     */

    
    SgImage(Device &device,
            VkImage handle,
            const VkExtent3D &extent,
            VkFormat format,
            VkImageUsageFlags image_usage,
            VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT,
            VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D);

    ~SgImage();

    SgImage(SgImage &&other);

    SgImage(SgImage &other) = delete;

    
   
    void createVkImage(Device &device, VkImageViewType image_view_type = VK_IMAGE_VIEW_TYPE_2D,
                       VkImageCreateFlags flags = 0);

    std::vector<uint8_t> &getData();

    uint64_t getBufferSize() const;

    VkExtent3D getExtent() const;

    VkExtent2D getExtent2D() const;

    Image &getVkImage() const;

    ImageView &getVkImageView() const;

    VkFormat getFormat() const;

    const std::vector<std::vector<VkDeviceSize>> &getOffsets() const;

    const std::vector<Mipmap> &getMipMaps() const;

    uint32_t getLayers() const;

    void setLayers(uint32_t layers);

    void generateMipMap();

    void setExtent(const VkExtent3D & extent3D);

    void loadResources(const std::string &path);

private:

protected:
    friend class AstcImageHelper;

    std::unique_ptr<Image> vkImage{nullptr};

    std::unique_ptr<ImageView> vkImageView{nullptr};

    std::vector<uint8_t> data;

    VkFormat format;

    VkExtent3D mExtent3D;


    std::vector<Mipmap> mipMaps{{}};

    std::vector<std::vector<VkDeviceSize>> offsets;

    std::string name;

    Device &device;

protected:
    uint32_t layers{1};
};
