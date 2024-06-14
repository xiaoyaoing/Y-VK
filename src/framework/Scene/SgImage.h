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

    bool isInitialized() const {
        return extent.width != 0 && extent.height != 0 && extent.depth != 0;
    }
};

class SgImage {
public:
    /**
     * \brief load from hardware texture
     */
    SgImage(Device& device, const std::string& path);

    /**
     * \brief load from memory
     */
    SgImage(Device& device, const std::vector<uint8_t>& data, VkExtent3D extent, VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);
    SgImage(Device& device, std::vector<uint8_t>&& data, VkExtent3D extent, VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);

    /**
     * \brief create from image attribute 
     */
    SgImage(Device&               device,
            const std::string&    name,
            const VkExtent3D&     extent,
            VkFormat              format,
            VkImageUsageFlags     image_usage,
            VmaMemoryUsage        memory_usage,
            VkImageViewType       viewType,
            VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT,
            uint32_t              mip_levels   = 1,
            uint32_t              array_layers = 1,
            VkImageCreateFlags    flags        = 0);

    /**
     * \brief load from existing image.Mainly from swapChainImage
     */

    SgImage(Device&               device,
            VkImage               handle,
            const VkExtent3D&     extent,
            VkFormat              format,
            VkImageUsageFlags     image_usage,
            VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT,
            VkImageViewType       viewType     = VK_IMAGE_VIEW_TYPE_2D);

    ~SgImage();

    SgImage(SgImage&& other);

    SgImage(SgImage& other) = delete;

    void freeImageCpuData();

    void createVkImage(Device& device, uint32_t mipLevels = 0, VkImageViewType imageViewType = VK_IMAGE_VIEW_TYPE_2D, VkImageCreateFlags flags = 0);

    std::vector<uint8_t>& getData();

    uint64_t getBufferSize() const;

    VkExtent3D getExtent() const;

    VkExtent2D getExtent2D() const;

    Image& getVkImage() const;

    // ImageView &getVkImageView() const;

    ImageView& getVkImageView(VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_MAX_ENUM, VkFormat format = VK_FORMAT_UNDEFINED, uint32_t mip_level = 0, uint32_t base_array_layer = 0, uint32_t n_mip_levels = 0, uint32_t n_array_layers = 0) const;

    void createImageView(VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_MAX_ENUM, VkFormat format = VK_FORMAT_UNDEFINED, uint32_t mip_level = 0, uint32_t base_array_layer = 0, uint32_t n_mip_levels = 0, uint32_t n_array_layers = 0);

    VkFormat getFormat() const;

    const std::vector<std::vector<VkDeviceSize>>& getOffsets() const;

    const std::vector<Mipmap>& getMipMaps() const;

    uint32_t getArrayLayerCount() const;
    uint32_t getMipLevelCount() const;

    void setArrayLevelCount(uint32_t layers);

    void generateMipMapOnCpu();
    void generateMipMapOnGpu();

    void setExtent(const VkExtent3D& extent3D);

    void loadResources(const std::string& path);

    bool isCubeMap() const;

    bool needGenerateMipMapOnGpu() const;

private:
protected:
    void setIsCubeMap(bool _isCube);

    friend class AstcImageHelper;

    std::unordered_map<size_t, std::unique_ptr<ImageView>> vkImageViews{};
    std::unique_ptr<Image>                                 vkImage{nullptr};

    size_t firstImageViewHash{0};

    //Attributes to init when load resources
    std::vector<uint8_t> mData{};
    VkFormat             format{};
    VkExtent3D           mExtent3D{0, 0, 1};

    //Attention: size = layer * level
    //layer 0 mipmap 0, layer1 mipmap0 layer2 mipmap0 ...
    std::vector<Mipmap>                    mipMaps{{}};
    std::vector<std::vector<VkDeviceSize>> offsets;
    bool                                   mIsCubeMap{false};
    bool                                   needGenerateMipMap{true};

    std::string name;

    Device& device;

    std::string filePath;

protected:
    uint32_t layers{1};
};
