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

#include "ctpl_stl.h"
#include "imgui.h"
#include "Core/ResourceCachingHelper.h"
#include "Images/AstcImageHelper.h"
#include "Images/KtxFormat.h"
#include "Core/Buffer.h"
#include "IO/ImageIO.h"
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

    auto& mipmap         = callback_data->mipmaps->at(callback_data->texture->numFaces * mipLevel + face);
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
    //hashValue = view_type << 2 | format << 4 | mip_level << 10 | base_array_layer << 16 | n_mip_levels << 20 | n_array_layers << 24;

    std::hash<std::underlying_type<VkImageViewType>::type> hasher;

    glm::detail::hash_combine(hashValue, hasher(view_type));
    glm::detail::hash_combine(hashValue, static_cast<std::underlying_type<VkFormat>::type>(format));
    glm::detail::hash_combine(hashValue, mip_level);
    glm::detail::hash_combine(hashValue, base_array_layer);
    glm::detail::hash_combine(hashValue, n_mip_levels);
    glm::detail::hash_combine(hashValue, n_array_layers);
    return hashValue;
}

VkImageType GetImageType(VkImageViewType viewType) {
    switch (viewType) {
        case VK_IMAGE_VIEW_TYPE_1D:
            return VK_IMAGE_TYPE_1D;
        case VK_IMAGE_VIEW_TYPE_2D:
            return VK_IMAGE_TYPE_2D;
        case VK_IMAGE_VIEW_TYPE_3D:
            return VK_IMAGE_TYPE_3D;
        case VK_IMAGE_VIEW_TYPE_CUBE:
            return VK_IMAGE_TYPE_3D;
        case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
            return VK_IMAGE_TYPE_1D;
        case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
            return VK_IMAGE_TYPE_2D;
        case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
            return VK_IMAGE_TYPE_2D;
        default:
            return VK_IMAGE_TYPE_MAX_ENUM;
    }
}

void SgImage::freeImageCpuData() {
    mData.clear();
}
void SgImage::createVkImage(Device& device, uint32_t mipLevels, VkImageViewType imageViewType, VkImageCreateFlags flags) {
    assert(vkImage == nullptr && "Image has been created");
    setIsCubeMap(imageViewType == VK_IMAGE_VIEW_TYPE_CUBE | isCubeMap());
    if (isCubeMap())
        flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    //
    if(needGenerateMipMap) {
        mipLevels = std::log2(std::max(mExtent3D.width, mExtent3D.height)) + 1;
    }
    else {
        mipLevels = getMipLevelCount();
    }
   // mipLevels = getMipLevelCount() == 1 ? std::log2(std::max(mExtent3D.width, mExtent3D.height)) + 1 : getMipLevelCount();
   // if (!needGenerateMipMap && getMipLevelCount()!=1) mipLevels = 1;

    vkImage = std::make_unique<Image>(device, mExtent3D, format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_SAMPLE_COUNT_1_BIT, mipLevels, layers, flags);
    createImageView();
}

SgImage::SgImage(Device& device, const std::string& path) : device(device), filePath(path) {
    loadResources(path);
}
SgImage::SgImage(Device& device, const std::vector<uint8_t>& data, VkExtent3D extent, VkImageViewType viewType, VkFormat format) : device(device), mData(data), format(format) {
    setIsCubeMap(viewType == VK_IMAGE_VIEW_TYPE_CUBE);
    setExtent(extent);
}
SgImage::SgImage(Device& device, std::vector<uint8_t>&& data, VkExtent3D extent, VkImageViewType viewType, VkFormat format) : device(device), mData(std::move(data)), format(format) {
    setIsCubeMap(viewType == VK_IMAGE_VIEW_TYPE_CUBE);
    setExtent(extent);
}

SgImage::~SgImage() {
    // if(vkImage)delete vkImage.get();
    // if(vkImageView) delete vkImageView.get();
}

SgImage::SgImage(SgImage&& other) : vkImage(std::move(other.vkImage)), vkImageViews(std::move(other.vkImageViews)),
                                    mData(std::move(other.mData)),
                                    format(other.format), mExtent3D(other.mExtent3D),
                                    mipMaps(std::move(other.mipMaps)), offsets(std::move(other.offsets)), mIsCubeMap(other.mIsCubeMap),
                                    name(std::move(other.name)),
                                    device(other.device), layers(other.layers) {
    other.vkImage = nullptr;
}

SgImage::SgImage(Device& device, const std::string& name, const VkExtent3D& extent, VkFormat format, VkImageUsageFlags image_usage, VmaMemoryUsage memory_usage, VkImageViewType viewType, VkSampleCountFlagBits sample_count, uint32_t mipLevels, uint32_t array_layers, VkImageCreateFlags flags) : format(format), mExtent3D(extent), name(name), device(device), layers(array_layers) {
    setIsCubeMap(viewType == VK_IMAGE_VIEW_TYPE_CUBE);
    if (isCubeMap())
        flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    vkImage = std::make_unique<Image>(device, extent, format, image_usage, memory_usage, sample_count, mipLevels, array_layers, flags);
    DebugUtils::SetObjectName(device.getHandle(),reinterpret_cast<uint64_t>(vkImage->getHandle()),VK_OBJECT_TYPE_IMAGE,name);
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

    auto extent_string       = std::to_string(extent.width) + "x" + std::to_string(extent.height) + "x" + std::to_string(extent.depth);
    auto format_string       = std::to_string(format);
    auto image_usage_string  = std::to_string(image_usage);
    auto memory_usage_string = std::to_string(memory_usage);
    auto sample_count_string = std::to_string(sample_count);
    auto mipLevels_string    = std::to_string(mipLevels);
    auto array_layers_string = std::to_string(array_layers);
    auto flags_string        = std::to_string(flags);
    LOGI("SgImage::SgImage: name = {}, extent = {}, format = {}, image_usage = {}, memory_usage = {}, sample_count = {}, mipLevels = {}, array_layers = {}, flags = {}", name, extent_string, format_string, image_usage_string, memory_usage_string, sample_count_string, mipLevels_string, array_layers_string, flags_string);
}

uint64_t SgImage::getBufferSize() const {
    return mData.size();
}

std::vector<uint8_t>& SgImage::getData() {
    return mData;
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

VkImageViewType GetViewType(VkImageType type, uint32_t layerCount) {
    switch (type) {
        case VK_IMAGE_TYPE_1D:
            return VK_IMAGE_VIEW_TYPE_1D;
        case VK_IMAGE_TYPE_2D:
            if (layerCount >= 6)
                return VK_IMAGE_VIEW_TYPE_CUBE;
            else if (layerCount > 1)
                return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            else
                return VK_IMAGE_VIEW_TYPE_2D;
        case VK_IMAGE_TYPE_3D:
            return VK_IMAGE_VIEW_TYPE_3D;
        default:
            return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
    }
}

ImageView& SgImage::getVkImageView(VkImageViewType view_type, VkFormat format, uint32_t mip_level, uint32_t base_array_layer, uint32_t n_mip_levels, uint32_t n_array_layers) const {

    if (view_type == VK_IMAGE_VIEW_TYPE_MAX_ENUM) {
        view_type = GetViewType(vkImage->getImageType(), vkImage->getArrayLayerCount());
    }
    if (format == VK_FORMAT_UNDEFINED) {
        format = vkImage->getFormat();
    }
    if (n_mip_levels == 0)
        n_mip_levels = getMipLevelCount();
    if (n_array_layers == 0)
        n_array_layers = layers;
    size_t hashValue = ImageViewHash(view_type, format, mip_level, base_array_layer, n_mip_levels, n_array_layers);
    if (!vkImageViews.contains(hashValue)) {
        LOGI("No image view found for hash value: {} created", hashValue)
        //may be ugly
        const_cast<std::map<size_t, std::unique_ptr<ImageView>>&>(vkImageViews)[hashValue] = std::make_unique<ImageView>(*vkImage, view_type, format, mip_level, base_array_layer, n_mip_levels, n_array_layers);
    }
    return *vkImageViews.at(hashValue);
}

void SgImage::createImageView(VkImageViewType view_type, VkFormat format, uint32_t mip_level, uint32_t base_array_layer, uint32_t n_mip_levels, uint32_t n_array_layers) {
    if (view_type == VK_IMAGE_VIEW_TYPE_MAX_ENUM) {
        view_type = GetViewType(vkImage->getImageType(), vkImage->getArrayLayerCount());
    }
    if (format == VK_FORMAT_UNDEFINED) {
        format = vkImage->getFormat();
    }
    if (n_mip_levels == 0)
        n_mip_levels = vkImage->getMipLevelCount();
    if (n_array_layers == 0)
        n_array_layers = layers;
    size_t hashValue = ImageViewHash(view_type, format, mip_level, base_array_layer, n_mip_levels, n_array_layers);
    if (vkImageViews.contains(hashValue)) {
        LOGW("Image view already created for hash value: {}", hashValue);
    }
    vkImageViews[hashValue] = std::make_unique<ImageView>(*vkImage, view_type, format, mip_level, base_array_layer, n_mip_levels, n_array_layers);
    DebugUtils::SetObjectName(device.getHandle(),reinterpret_cast<uint64_t>(vkImageViews[hashValue]->getHandle()),VK_OBJECT_TYPE_IMAGE_VIEW,name);
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

uint32_t SgImage::getArrayLayerCount() const {
    return layers;
}
uint32_t SgImage::getMipLevelCount() const {
    if (vkImage)
        return vkImage->getMipLevelCount();
    return mipMaps.size() / layers;
}

void SgImage::setArrayLevelCount(uint32_t layers) {
    SgImage::layers = layers;
}

void SgImage::generateMipMapOnCpu() {
    if (mipMaps.size() > 1 && mipMaps[0].isInitialized())
        return;
    if (!needGenerateMipMap) return;
    assert(mipMaps.size() == 1 && "Mipmaps already generated");

    for (int i = 0; i < layers; i++) {
        auto extent      = getExtent();
        auto next_width  = std::max<uint32_t>(1u, extent.width / 2);
        auto next_height = std::max<uint32_t>(1u, extent.height / 2);
        auto channels    = 4;
        auto next_size   = next_width * next_height * channels;
        auto level       = 0;
        while (true) {
            // Make space for next mipmap
            auto old_size = toUint32(mData.size());
            mData.resize(old_size + next_size);

            auto& prev_mipmap = mipMaps.back();
            // Update mipmaps
            Mipmap next_mipmap{};
            next_mipmap.level  = level++;
            next_mipmap.offset = old_size;
            next_mipmap.extent = {next_width, next_height, 1u};

            //todo

            stbir_resize_uint8(mData.data() + prev_mipmap.offset, prev_mipmap.extent.width, prev_mipmap.extent.height, 0, mData.data() + next_mipmap.offset, next_mipmap.extent.width, next_mipmap.extent.height, 0, channels);
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
}
void SgImage::generateMipMapOnGpu() {
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
        if (rawData == nullptr) {
            return;
        }
        comp  = 4;
        mData = {rawData, rawData + w * h * comp};
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

        mData  = std::vector<uint8_t>(fontData, fontData + texHeight * texWidth * 4);
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

        mipMaps.resize(ktxTexture->numLevels * ktxTexture->numFaces);

        CallbackData callbackData{
            .texture = ktxTexture,
            .mipmaps = &mipMaps,
        };
        result = ktxTexture_IterateLevelFaces(ktxTexture, callBack, &callbackData);
        assert(result == KTX_SUCCESS);
        layers = ktxTexture->numLayers;
        setIsCubeMap(ktxTexture->isCubemap);
        if (ktxTexture->isCubemap) {
            layers = 6;
        }

        ktx_size_t pixelsMulChannels;
        result = ktxTexture_GetImageOffset(ktxTexture, 0, 0, 0, &pixelsMulChannels);
        assert(result == KTX_SUCCESS);

        mData = {ktxTextureData, ktxTextureData + ktxTextureSize};

        format = GetFormatFromOpenGLInternalFormat(ktxTexture->glInternalformat);

        if (layers > 1) {
            for (uint32_t layer = 0; layer < layers; layer++) {
                std::vector<VkDeviceSize> layerOffsets{};
                for (uint32_t level = 0; level < ktxTexture->numLevels; level++) {
                    ktx_size_t offset;

                    if (ktxTexture->isCubemap)
                        result = ktxTexture_GetImageOffset(ktxTexture, level, 0, layer, &offset);

                    else
                        result = ktxTexture_GetImageOffset(ktxTexture, level, layer, 0, &offset);

                    assert(result == KTX_SUCCESS);
                    layerOffsets.push_back(static_cast<VkDeviceSize>(offset));
                }
                offsets.push_back(layerOffsets);
            }
        }
        needGenerateMipMap = ktxTexture->numLevels == 1;
    } else if (ext == "hdr") {
        int    component;
        float* pixels = stbi_loadf(path.c_str(), reinterpret_cast<int*>(&mExtent3D.width), reinterpret_cast<int*>(&mExtent3D.height), &component, STBI_rgb_alpha);
        size_t size   = mExtent3D.width * mExtent3D.height * 4 * sizeof(float);
        setExtent({toUint32(mExtent3D.width), toUint32(mExtent3D.height), 1});
        mData.resize(size);
        memcpy(mData.data(), pixels, size);
        stbi_image_free(pixels);
        format             = VK_FORMAT_R32G32B32A32_SFLOAT;
        needGenerateMipMap = false;
    }
    else if (ext == "dds" || ext == "exr") {
        auto desc = ImageIO::loadImage(path);
        setExtent(desc.extent);
        mData              = std::move(desc.data);
        format             = desc.format;
        needGenerateMipMap = desc.needGenerateMipmaps;
        mipMaps            = desc.mipmaps;
    } else {
        LOGE("Unsupported image format: {}", ext);
    }

    if (isAstc(format)) {
        if (!device.isImageFormatSupported(format)) {
            LOGW("ASTC not supported: decoding {}", name);
            AstcImageHelper::decodeAstcImage(*this);
        }
    }
}

bool SgImage::isCubeMap() const {
    return mIsCubeMap;
}
bool SgImage::needGenerateMipMapOnGpu() const {
    return needGenerateMipMap;
}
void SgImage::setIsCubeMap(bool _isCube) {
    mIsCubeMap = _isCube;
}

SgImage::SgImage(Device& device, VkImage handle, const VkExtent3D& extent, VkFormat format, VkImageUsageFlags image_usage, VkSampleCountFlagBits sample_count, VkImageViewType viewType) : device(device) {
    VkImageCreateFlags flags = 0;
    vkImage                  = std::make_unique<Image>(device, handle, extent, format, image_usage, sample_count);
    createImageView(viewType);
    this->format = format;
    setExtent(extent);
}