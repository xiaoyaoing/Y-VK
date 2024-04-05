//
// Created by pc on 2023/8/18.
//

#include "SgImage.h"
#include "Core/Images/Image.h"

#include "Core/Images/ImageView.h"

#include "Core/Device/Device.h"
#include "Common/FIleUtils.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
// #include <ext/ktx/lib/vk_format.h>
#include <ktx.h>
#include <stb_image.h>
#include <stb_image_resize.h>
// #include <stb_image_resize.h>

#include "imgui.h"
#include "Core/ResourceCachingHelper.h"
#include "Images/AstcImageHelper.h"
#include "Images/KtxFormat.h"

struct CallbackData final {
    ktxTexture*          texture;
    std::vector<Mipmap>* mipmaps;
};

bool isAstc(const VkFormat format) {
    return (format == VK_FORMAT_ASTC_4x4_UNORM_BLOCK ||
            format == VK_FORMAT_ASTC_4x4_SRGB_BLOCK ||
            format == VK_FORMAT_ASTC_5x4_UNORM_BLOCK ||
            format == VK_FORMAT_ASTC_5x4_SRGB_BLOCK ||
            format == VK_FORMAT_ASTC_5x5_UNORM_BLOCK ||
            format == VK_FORMAT_ASTC_5x5_SRGB_BLOCK ||
            format == VK_FORMAT_ASTC_6x5_UNORM_BLOCK ||
            format == VK_FORMAT_ASTC_6x5_SRGB_BLOCK ||
            format == VK_FORMAT_ASTC_6x6_UNORM_BLOCK ||
            format == VK_FORMAT_ASTC_6x6_SRGB_BLOCK ||
            format == VK_FORMAT_ASTC_8x5_UNORM_BLOCK ||
            format == VK_FORMAT_ASTC_8x5_SRGB_BLOCK ||
            format == VK_FORMAT_ASTC_8x6_UNORM_BLOCK ||
            format == VK_FORMAT_ASTC_8x6_SRGB_BLOCK ||
            format == VK_FORMAT_ASTC_8x8_UNORM_BLOCK ||
            format == VK_FORMAT_ASTC_8x8_SRGB_BLOCK ||
            format == VK_FORMAT_ASTC_10x5_UNORM_BLOCK ||
            format == VK_FORMAT_ASTC_10x5_SRGB_BLOCK ||
            format == VK_FORMAT_ASTC_10x6_UNORM_BLOCK ||
            format == VK_FORMAT_ASTC_10x6_SRGB_BLOCK ||
            format == VK_FORMAT_ASTC_10x8_UNORM_BLOCK ||
            format == VK_FORMAT_ASTC_10x8_SRGB_BLOCK ||
            format == VK_FORMAT_ASTC_10x10_UNORM_BLOCK ||
            format == VK_FORMAT_ASTC_10x10_SRGB_BLOCK ||
            format == VK_FORMAT_ASTC_12x10_UNORM_BLOCK ||
            format == VK_FORMAT_ASTC_12x10_SRGB_BLOCK ||
            format == VK_FORMAT_ASTC_12x12_UNORM_BLOCK ||
            format == VK_FORMAT_ASTC_12x12_SRGB_BLOCK);
}

static KTX_error_code KTXAPIENTRY callBack(int mipLevel, int face, int width, int height, int depth, ktx_uint32_t faceLodSize, void* pixels, void* userdata) {
    auto* callback_data = reinterpret_cast<CallbackData*>(userdata);
    assert(static_cast<size_t>(mipLevel) < callback_data->mipmaps->size() && "Not enough space in the mipmap vector");

    ktx_size_t mipmap_offset = 0;
    auto       result        = ktxTexture_GetImageOffset(callback_data->texture, mipLevel, 0, face, &mipmap_offset);
    if (result != KTX_SUCCESS) {
        return result;
    }

    auto& mipmap         = callback_data->mipmaps->at(mipLevel);
    mipmap.level         = mipLevel;
    mipmap.offset        = toUint32(mipmap_offset);
    mipmap.extent.width  = width;
    mipmap.extent.height = height;
    mipmap.extent.depth  = depth;

    return KTX_SUCCESS;
}

static size_t ImageViewHash(const VkImageViewType& view_type, const VkFormat& format, const uint32_t& mip_level, const uint32_t& base_array_layer, const uint32_t& n_mip_levels, const uint32_t& n_array_layers) {
    size_t hashValue = 0;

    //todo optimize this
    hashValue = view_type << 2 | format << 4 | mip_level << 10 | base_array_layer << 16 | n_mip_levels << 20 | n_array_layers << 24;

    // std::hash<std::underlying_type<VkImageViewType>::type> hasher;
    //
    // glm::detail::hash_combine( hashValue,hasher(view_type));
    // glm::detail::hash_combine(hashValue, static_cast<std::underlying_type<VkFormat>::type>(format));
    // glm::detail::hash_combine(hashValue, std::hash<uint32_t>(mip_level));
    // glm::detail::hash_combine(hashValue, base_array_layer);
    // glm::detail::hash_combine(hashValue, n_mip_levels);
    // glm::detail::hash_combine(hashValue, n_array_layers);
    return hashValue;
}

void SgImage::createVkImage(Device& device, VkImageViewType imageViewType, VkImageCreateFlags flags) {
    assert(vkImage == nullptr && "Image has been created");
    //   assert(vkImageView == nullptr && "ImageView has been created");
    vkImage = std::make_unique<Image>(device, mExtent3D, format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_SAMPLE_COUNT_1_BIT, toUint32(mipMaps.size()), layers, flags);

    createImageView();
}

SgImage::SgImage(Device& device, const std::string& path, VkImageViewType viewType) : device(device) {
    loadResources(path);
    //createVkImage(device, viewType);
}

SgImage::~SgImage() {
    // if(vkImage)delete vkImage.get();
    // if(vkImageView) delete vkImageView.get();
}

SgImage::SgImage(SgImage&& other) : vkImage(std::move(other.vkImage)), vkImageViews(std::move(other.vkImageViews)),
                                    data(std::move(other.data)),
                                    format(other.format), mExtent3D(other.mExtent3D),
                                    mipMaps(std::move(other.mipMaps)), offsets(std::move(other.offsets)),
                                    name(std::move(other.name)),
                                    device(other.device), layers(other.layers) {
    other.vkImage = nullptr;
}

SgImage::SgImage(Device& device, const std::string& name, const VkExtent3D& extent, VkFormat format, VkImageUsageFlags image_usage, VmaMemoryUsage memory_usage, VkImageViewType viewType, VkSampleCountFlagBits sample_count, uint32_t mipLevels, uint32_t array_layers, VkImageCreateFlags flags) : format(format), mExtent3D(extent), name(name), device(device) {
    vkImage = std::make_unique<Image>(device, extent, format, image_usage, memory_usage, sample_count, mipLevels, array_layers, flags);
    createImageView();


    size_t hash;
    hash_combine(hash, name);
    hash_combine(hash, extent);
    hash_combine(hash, format);
    hash_combine(hash, image_usage);
    hash_combine(hash, memory_usage);
    hash_combine(hash, viewType);
    hash_combine(hash, sample_count);
    hash_combine(hash, mipLevels);
    hash_combine(hash, array_layers);
    hash_combine(hash, flags);
 //   LOGI("Creating  {0} with hash {1} ", typeid(SgImage).name(), hash);
    
    this->format    = format;
    this->mExtent3D = extent;
}

uint64_t SgImage::getBufferSize() const {
    return data.size();
}

std::vector<uint8_t>& SgImage::getData() {
    return data;
}

VkExtent3D SgImage::getExtent() const {
    return mExtent3D;
}

VkExtent2D SgImage::getExtent2D() const {
    return {mExtent3D.width, mExtent3D.height};
}

Image& SgImage::getVkImage() const {
    return *vkImage;
}

VkImageViewType GetViewType(VkImageType type) {
    switch (type) {
        case VK_IMAGE_TYPE_1D:
            return VK_IMAGE_VIEW_TYPE_1D;
        case VK_IMAGE_TYPE_2D:
            return VK_IMAGE_VIEW_TYPE_2D;
        case VK_IMAGE_TYPE_3D:
            return VK_IMAGE_VIEW_TYPE_3D;
        default:
            return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
    }
}

ImageView& SgImage::getVkImageView(VkImageViewType view_type, VkFormat format, uint32_t mip_level, uint32_t base_array_layer, uint32_t n_mip_levels, uint32_t n_array_layers) const {

    if (view_type == VK_IMAGE_VIEW_TYPE_MAX_ENUM) {
        view_type = GetViewType(vkImage->getImageType());
    }
    if (format == VK_FORMAT_UNDEFINED) {
        format = vkImage->getFormat();
    }

    size_t hashValue = ImageViewHash(view_type, format, mip_level, base_array_layer, n_mip_levels, n_array_layers);

    if (!vkImageViews.contains(hashValue)) {
        LOGE("No image view found for hash value: {}", hashValue);
    }
    return *vkImageViews.at(hashValue);
}

void SgImage::createImageView(VkImageViewType view_type, VkFormat format, uint32_t mip_level, uint32_t base_array_layer, uint32_t n_mip_levels, uint32_t n_array_layers) {
    if (view_type == VK_IMAGE_VIEW_TYPE_MAX_ENUM) {
        view_type = GetViewType(vkImage->getImageType());
    }
    if (format == VK_FORMAT_UNDEFINED) {
        format = vkImage->getFormat();
    }
    size_t hashValue = ImageViewHash(view_type, format, mip_level, base_array_layer, n_mip_levels, n_array_layers);
    if (vkImageViews.contains(hashValue)) {
        LOGW("Image view already created for hash value: {}", hashValue);
    }
    vkImageViews[hashValue] = std::make_unique<ImageView>(*vkImage, view_type, format, mip_level, base_array_layer, n_mip_levels, n_array_layers);
}

VkFormat SgImage::getFormat() const {
    return format;
}

const std::vector<Mipmap>& SgImage::getMipMaps() const {
    return mipMaps;
}

const std::vector<std::vector<VkDeviceSize>>& SgImage::getOffsets() const {
    return offsets;
}

uint32_t SgImage::getLayers() const {
    return layers;
}

void SgImage::setLayers(uint32_t layers) {
    SgImage::layers = layers;
}

void SgImage::generateMipMap() {
    if (mipMaps.size() > 1)
        return;
    assert(mipMaps.size() == 1 && "Mipmaps already generated");

    auto extent      = getExtent();
    auto next_width  = std::max<uint32_t>(1u, extent.width / 2);
    auto next_height = std::max<uint32_t>(1u, extent.height / 2);
    auto channels    = 4;
    auto next_size   = next_width * next_height * channels;

    while (true) {
        // Make space for next mipmap
        auto old_size = toUint32(data.size());
        data.resize(old_size + next_size);

        auto& prev_mipmap = mipMaps.back();
        // Update mipmaps
        Mipmap next_mipmap{};
        next_mipmap.level  = prev_mipmap.level + 1;
        next_mipmap.offset = old_size;
        next_mipmap.extent = {next_width, next_height, 1u};

        //todo

        stbir_resize_uint8(data.data() + prev_mipmap.offset, prev_mipmap.extent.width, prev_mipmap.extent.height, 0, data.data() + next_mipmap.offset, next_mipmap.extent.width, next_mipmap.extent.height, 0, channels);
        // Fill next mipmap memory
        //            stbir_resize_uint8(data.data() + prev_mipmap.offset, prev_mipmap.extent.width, prev_mipmap.extent.height, 0,
        //                               data.data() + next_mipmap.offset, next_mipmap.extent.width, next_mipmap.extent.height, 0,
        //                               channels);

        mipMaps.emplace_back(std::move(next_mipmap));

        // Next mipmap values
        next_width  = std::max<uint32_t>(1u, next_width / 2);
        next_height = std::max<uint32_t>(1u, next_height / 2);
        next_size   = next_width * next_height * channels;

        if (next_width <= 1 && next_height <= 1) {
            break;
        }
    }
}

void SgImage::setExtent(const VkExtent3D& extent3D) {
    this->mExtent3D   = extent3D;
    mipMaps[0].extent = extent3D;
}

void SgImage::loadResources(const std::string& path) {
    auto ext = FileUtils::getFileExt(path);
    if (ext == "jpg" || ext == "png") {
        int  w, h, comp;
        auto rawData = stbi_load(path.c_str(), &w, &h, &comp, 4);
        comp         = 4;
        data         = {rawData, rawData + w * h * comp};
        stbi_image_free(rawData);

        setExtent({toUint32(w), toUint32(h), 1});
        format = VK_FORMAT_R8G8B8A8_SRGB;
    } else if (ext == "ttf") {
        assert(FileUtils::getFileExt(path) == "ttf");
        ImGuiIO& io = ImGui::GetIO();
        // io.Fonts->AddFontFromFileTTF(path.c_str(), 16.f);

        unsigned char* fontData;
        int            texWidth, texHeight;
        io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);

        data   = std::vector<uint8_t>(fontData, fontData + texHeight * texWidth * 4);
        format = VK_FORMAT_R8G8B8A8_UNORM;
        setExtent({toUint32(texWidth), toUint32(texHeight), 1});
    } else if (ext == "ktx") {
        ktxTexture* ktxTexture;
        ktxResult   result = ktxTexture_CreateFromNamedFile(path.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);
        assert(result == KTX_SUCCESS);
        //   this->extent3D = VkExtent3D{ktxTexture->baseWidth, ktxTexture->baseHeight, 1};
        setExtent({ktxTexture->baseWidth, ktxTexture->baseHeight, 1});

        ktx_uint8_t* ktxTextureData = ktxTexture_GetData(ktxTexture);
        ktx_size_t   ktxTextureSize = ktxTexture_GetSize(ktxTexture);

        mipMaps.resize(ktxTexture->numLevels);

        CallbackData callbackData{
            .texture = ktxTexture,
            .mipmaps = &mipMaps,
        };
        result = ktxTexture_IterateLevels(ktxTexture, callBack, &callbackData);
        assert(result == KTX_SUCCESS);

        // for(uint32_t i = 0; i < ktxTexture->numLevels; i++)
        // {
        //     mipMaps[i].offset = ktxTextureSize
        // }

        layers = ktxTexture->numLayers;

        ktx_size_t pixelsMulChannels;
        result = ktxTexture_GetImageOffset(ktxTexture, 0, 0, 0, &pixelsMulChannels);
        assert(result == KTX_SUCCESS);

        data = {ktxTextureData, ktxTextureData + ktxTextureSize};

        format = GetFormatFromOpenGLInternalFormat(ktxTexture->glInternalformat);

        if (layers > 1) {
            for (uint32_t layer = 0; layer < layers; layer++) {
                std::vector<VkDeviceSize> layerOffsets{};
                for (uint32_t level = 0; level < mipMaps.size(); level++) {
                    ktx_size_t offset;

                    result = ktxTexture_GetImageOffset(ktxTexture, level, layer, 0, &offset);
                    assert(result == KTX_SUCCESS);
                    layerOffsets.push_back(static_cast<VkDeviceSize>(offset));
                }
                offsets.push_back(layerOffsets);
            }
        }
    }

    if (isAstc(format)) {
        if (!device.isImageFormatSupported(format)) {
            LOGW("ASTC not supported: decoding {}", name);
            AstcImageHelper::decodeAstcImage(*this);
            generateMipMap();
        }
    }
}

SgImage::SgImage(Device& device, VkImage handle, const VkExtent3D& extent, VkFormat format, VkImageUsageFlags image_usage, VkSampleCountFlagBits sample_count, VkImageViewType viewType) : device(device) {
    vkImage = std::make_unique<Image>(device, handle, extent, format, image_usage, sample_count);
    createImageView();
    this->format = format;
    setExtent(extent);
}