#include "ImageIO.h"
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "ctpl_stl.h"
#include "Common/FIleUtils.h"
#include "Common/Log.h"

#include <stb_image_write.h>
#include <vector>

#include <dds.hpp>

VkFormat ConvertFormatFromDxgiFormat(DXGI_FORMAT format, bool alpha_flag) {
    switch (format) {
        case DXGI_FORMAT_BC1_UNORM: {
            if (alpha_flag)
                return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
            else
                return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
        }
        case DXGI_FORMAT_BC1_UNORM_SRGB: {
            if (alpha_flag)
                return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
            else
                return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
        }

        case DXGI_FORMAT_BC2_UNORM:
            return VK_FORMAT_BC2_UNORM_BLOCK;
        case DXGI_FORMAT_BC2_UNORM_SRGB:
            return VK_FORMAT_BC2_SRGB_BLOCK;
        case DXGI_FORMAT_BC3_UNORM:
            return VK_FORMAT_BC3_UNORM_BLOCK;
        case DXGI_FORMAT_BC3_UNORM_SRGB:
            return VK_FORMAT_BC3_SRGB_BLOCK;
        case DXGI_FORMAT_BC4_UNORM:
            return VK_FORMAT_BC4_UNORM_BLOCK;
        case DXGI_FORMAT_BC4_SNORM:
            return VK_FORMAT_BC4_SNORM_BLOCK;
        case DXGI_FORMAT_BC5_UNORM:
            return VK_FORMAT_BC5_UNORM_BLOCK;
        case DXGI_FORMAT_BC5_SNORM:
            return VK_FORMAT_BC5_SNORM_BLOCK;
        case DXGI_FORMAT_BC7_UNORM:
            return VK_FORMAT_BC7_UNORM_BLOCK;
        case DXGI_FORMAT_BC7_UNORM_SRGB:
            return VK_FORMAT_BC7_SRGB_BLOCK;

        // 8-bit wide formats
        case DXGI_FORMAT_R8_UNORM:
            return VK_FORMAT_R8_UNORM;
        case DXGI_FORMAT_R8_UINT:
            return VK_FORMAT_R8_UINT;
        case DXGI_FORMAT_R8_SNORM:
            return VK_FORMAT_R8_SNORM;
        case DXGI_FORMAT_R8_SINT:
            return VK_FORMAT_R8_SINT;

        // 16-bit wide formats
        case DXGI_FORMAT_R8G8_UNORM:
            return VK_FORMAT_R8G8_UNORM;
        case DXGI_FORMAT_R8G8_UINT:
            return VK_FORMAT_R8G8_SINT;
        case DXGI_FORMAT_R8G8_SNORM:
            return VK_FORMAT_R8G8_SNORM;
        case DXGI_FORMAT_R8G8_SINT:
            return VK_FORMAT_R8G8_SINT;

        case DXGI_FORMAT_R16_FLOAT:
            return VK_FORMAT_R16_SFLOAT;
        case DXGI_FORMAT_R16_UNORM:
            return VK_FORMAT_R16_UNORM;
        case DXGI_FORMAT_R16_UINT:
            return VK_FORMAT_R16_UINT;
        case DXGI_FORMAT_R16_SNORM:
            return VK_FORMAT_R16_SNORM;
        case DXGI_FORMAT_R16_SINT:
            return VK_FORMAT_R16_SINT;

        case DXGI_FORMAT_B5G5R5A1_UNORM:
            return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
        case DXGI_FORMAT_B5G6R5_UNORM:
            return VK_FORMAT_B5G6R5_UNORM_PACK16;
        case DXGI_FORMAT_B4G4R4A4_UNORM:
            return VK_FORMAT_B4G4R4A4_UNORM_PACK16;

        // 32-bit wide formats
        case DXGI_FORMAT_R8G8B8A8_UNORM:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            return VK_FORMAT_R8G8B8A8_SRGB;
        case DXGI_FORMAT_R8G8B8A8_UINT:
            return VK_FORMAT_R8G8B8A8_UINT;
        case DXGI_FORMAT_R8G8B8A8_SNORM:
            return VK_FORMAT_R8G8B8A8_SNORM;
        case DXGI_FORMAT_R8G8B8A8_SINT:
            return VK_FORMAT_R8G8B8A8_SINT;
        case DXGI_FORMAT_B8G8R8A8_UNORM:
            return VK_FORMAT_B8G8R8A8_UNORM;
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
            return VK_FORMAT_B8G8R8A8_SRGB;

        case DXGI_FORMAT_R16G16_FLOAT:
            return VK_FORMAT_R16G16_SFLOAT;
        case DXGI_FORMAT_R16G16_UNORM:
            return VK_FORMAT_R16G16_UNORM;
        case DXGI_FORMAT_R16G16_UINT:
            return VK_FORMAT_R16G16_UINT;
        case DXGI_FORMAT_R16G16_SNORM:
            return VK_FORMAT_R16G16_SNORM;
        case DXGI_FORMAT_R16G16_SINT:
            return VK_FORMAT_R16G16_SINT;

        case DXGI_FORMAT_R32_FLOAT:
            return VK_FORMAT_R32_SFLOAT;
        case DXGI_FORMAT_R32_UINT:
            return VK_FORMAT_R32_UINT;
        case DXGI_FORMAT_R32_SINT:
            return VK_FORMAT_R32_SINT;

        case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
            return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
        case DXGI_FORMAT_R10G10B10A2_UNORM:
            return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
        case DXGI_FORMAT_R10G10B10A2_UINT:
            return VK_FORMAT_A2B10G10R10_UINT_PACK32;
        case DXGI_FORMAT_R11G11B10_FLOAT:
            return VK_FORMAT_B10G11R11_UFLOAT_PACK32;

        // 64-bit wide formats
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        case DXGI_FORMAT_R16G16B16A16_SINT:
            return VK_FORMAT_R16G16B16A16_SINT;
        case DXGI_FORMAT_R16G16B16A16_UINT:
            return VK_FORMAT_R16G16B16A16_UINT;
        case DXGI_FORMAT_R16G16B16A16_UNORM:
            return VK_FORMAT_R16G16B16A16_UNORM;
        case DXGI_FORMAT_R16G16B16A16_SNORM:
            return VK_FORMAT_R16G16B16A16_SNORM;

        case DXGI_FORMAT_R32G32_FLOAT:
            return VK_FORMAT_R32G32_SFLOAT;
        case DXGI_FORMAT_R32G32_UINT:
            return VK_FORMAT_R32G32_UINT;
        case DXGI_FORMAT_R32G32_SINT:
            return VK_FORMAT_R32G32_SINT;

        // 96-bit wide formats
        case DXGI_FORMAT_R32G32B32_FLOAT:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case DXGI_FORMAT_R32G32B32_UINT:
            return VK_FORMAT_R32G32B32_UINT;
        case DXGI_FORMAT_R32G32B32_SINT:
            return VK_FORMAT_R32G32B32_SINT;

        // 128-bit wide formats
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case DXGI_FORMAT_R32G32B32A32_UINT:
            return VK_FORMAT_R32G32B32A32_UINT;
        case DXGI_FORMAT_R32G32B32A32_SINT:
            return VK_FORMAT_R32G32B32A32_SINT;

        case DXGI_FORMAT_R8G8_B8G8_UNORM:
        case DXGI_FORMAT_G8R8_G8B8_UNORM:
        case DXGI_FORMAT_YUY2:
        default:
            return VK_FORMAT_UNDEFINED;
    }
}

ImageIO::ImageDesc ImageIO::loadImage(const std::string& path) {
    auto ext = FileUtils::getFileExt(path);
    if (ext == "dds") {
        LOGI("Loading DDS image from {}", path.c_str());
        ImageDesc  desc;
        dds::Image image;
        auto       result = dds::readFile(path, &image);
        assert(result == dds::Success);
        desc.extent = {image.width, image.height, image.depth};
        desc.data.resize(image.data.size());
        memcpy(desc.data.data(), image.data.data(), image.data.size());
        desc.format              = ConvertFormatFromDxgiFormat(image.format, image.supportsAlpha);
        desc.needGenerateMipmaps = false;
        return desc;
    }
}
void ImageIO::saveImage(const std::string& path, std::shared_ptr<std::vector<uint8_t>> data, int width, int height, int channels, bool ldr, bool hdr) {
    ctpl::thread_pool pool(1);

    std::future<void> future = pool.push([path, ldr, hdr, width, height, data](int id) {
        if (ldr)
            ImageIO::saveLdr(path, data->data(), width, height, 4);
        if (hdr)
            ImageIO::saveHdr(path, data->data(), width, height, 4);
    });
}
void ImageIO::saveLdr(const std::string& path, void* data, int width, int height, int channels) {
    std::vector<uint8_t> image_data;
    image_data.resize(width * height * channels);
    memcpy(image_data.data(), data, image_data.size());
    LOGI("Saving image to {}", path.c_str());
    stbi_write_png(path.c_str(), width, height, channels, image_data.data(), width * channels);
}

void ImageIO::saveHdr(const std::string& path, void* data, int width, int height, int channels) {
}
