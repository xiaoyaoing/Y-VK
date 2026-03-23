#include "ImageIO.h"
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "ctpl_stl.h"
#include "Common/FIleUtils.h"
#include "Common/Log.h"

#include <stb_image_write.h>
#include <vector>

// #include <dds.hpp>

#define TINYDDSLOADER_IMPLEMENTATION
#include "tinyddsloader.h"

#include <ext/tinyexr/tinyexr.h>

namespace tinyddsloader {
    VkFormat ConvertFormatFromDxgiFormat(DDSFile::DXGIFormat format) {
        switch (format) {
            case DDSFile::DXGIFormat::BC1_UNorm: {
                return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
            }
            case DDSFile::DXGIFormat::BC1_UNorm_SRGB: {
                return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
            }

            case DDSFile::DXGIFormat::BC2_UNorm:
                return VK_FORMAT_BC2_UNORM_BLOCK;
            case DDSFile::DXGIFormat::BC2_UNorm_SRGB:
                return VK_FORMAT_BC2_SRGB_BLOCK;
            case DDSFile::DXGIFormat::BC3_UNorm:
                return VK_FORMAT_BC3_UNORM_BLOCK;
            case DDSFile::DXGIFormat::BC3_UNorm_SRGB:
                return VK_FORMAT_BC3_SRGB_BLOCK;
            case DDSFile::DXGIFormat::BC4_UNorm:
                return VK_FORMAT_BC4_UNORM_BLOCK;
            case DDSFile::DXGIFormat::BC4_SNorm:
                return VK_FORMAT_BC4_SNORM_BLOCK;
            case DDSFile::DXGIFormat::BC5_UNorm:
                return VK_FORMAT_BC5_UNORM_BLOCK;
            case DDSFile::DXGIFormat::BC5_SNorm:
                return VK_FORMAT_BC5_SNORM_BLOCK;
            case DDSFile::DXGIFormat::BC7_UNorm:
                return VK_FORMAT_BC7_UNORM_BLOCK;
            case DDSFile::DXGIFormat::BC7_UNorm_SRGB:
                return VK_FORMAT_BC7_SRGB_BLOCK;

            // 8-bit wide formats
            case DDSFile::DXGIFormat::R8_UNorm:
                return VK_FORMAT_R8_UNORM;
            case DDSFile::DXGIFormat::R8_UInt:
                return VK_FORMAT_R8_UINT;
            case DDSFile::DXGIFormat::R8_SNorm:
                return VK_FORMAT_R8_SNORM;
            case DDSFile::DXGIFormat::R8_SInt:
                return VK_FORMAT_R8_SINT;

            // 16-bit wide formats
            case DDSFile::DXGIFormat::R8G8_UNorm:
                return VK_FORMAT_R8G8_UNORM;
            case DDSFile::DXGIFormat::R8G8_UInt:
                return VK_FORMAT_R8G8_SINT;
            case DDSFile::DXGIFormat::R8G8_SNorm:
                return VK_FORMAT_R8G8_SNORM;
            case DDSFile::DXGIFormat::R8G8_SInt:
                return VK_FORMAT_R8G8_SINT;

            case DDSFile::DXGIFormat::R16_Float:
                return VK_FORMAT_R16_SFLOAT;
            case DDSFile::DXGIFormat::R16_UNorm:
                return VK_FORMAT_R16_UNORM;
            case DDSFile::DXGIFormat::R16_UInt:
                return VK_FORMAT_R16_UINT;
            case DDSFile::DXGIFormat::R16_SNorm:
                return VK_FORMAT_R16_SNORM;
            case DDSFile::DXGIFormat::R16_SInt:
                return VK_FORMAT_R16_SINT;

            case DDSFile::DXGIFormat::B5G5R5A1_UNorm:
                return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
            case DDSFile::DXGIFormat::B5G6R5_UNorm:
                return VK_FORMAT_B5G6R5_UNORM_PACK16;
            case DDSFile::DXGIFormat::B4G4R4A4_UNorm:
                return VK_FORMAT_B4G4R4A4_UNORM_PACK16;

            // 32-bit wide formats
            case DDSFile::DXGIFormat::R8G8B8A8_UNorm:
                return VK_FORMAT_R8G8B8A8_UNORM;
            case DDSFile::DXGIFormat::R8G8B8A8_UNorm_SRGB:
                return VK_FORMAT_R8G8B8A8_SRGB;
            case DDSFile::DXGIFormat::R8G8B8A8_UInt:
                return VK_FORMAT_R8G8B8A8_UINT;
            case DDSFile::DXGIFormat::R8G8B8A8_SNorm:
                return VK_FORMAT_R8G8B8A8_SNORM;
            case DDSFile::DXGIFormat::R8G8B8A8_SInt:
                return VK_FORMAT_R8G8B8A8_SINT;
            case DDSFile::DXGIFormat::B8G8R8A8_UNorm:
                return VK_FORMAT_B8G8R8A8_UNORM;
            case DDSFile::DXGIFormat::B8G8R8A8_UNorm_SRGB:
                return VK_FORMAT_B8G8R8A8_SRGB;

            case DDSFile::DXGIFormat::R16G16_Float:
                return VK_FORMAT_R16G16_SFLOAT;
            case DDSFile::DXGIFormat::R16G16_UNorm:
                return VK_FORMAT_R16G16_UNORM;
            case DDSFile::DXGIFormat::R16G16_UInt:
                return VK_FORMAT_R16G16_UINT;
            case DDSFile::DXGIFormat::R16G16_SNorm:
                return VK_FORMAT_R16G16_SNORM;
            case DDSFile::DXGIFormat::R16G16_SInt:
                return VK_FORMAT_R16G16_SINT;

            case DDSFile::DXGIFormat::R32_Float:
                return VK_FORMAT_R32_SFLOAT;
            case DDSFile::DXGIFormat::R32_UInt:
                return VK_FORMAT_R32_UINT;
            case DDSFile::DXGIFormat::R32_SInt:
                return VK_FORMAT_R32_SINT;

            case DDSFile::DXGIFormat::R9G9B9E5_SHAREDEXP:
                return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
            case DDSFile::DXGIFormat::R10G10B10A2_UNorm:
                return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
            case DDSFile::DXGIFormat::R10G10B10A2_UInt:
                return VK_FORMAT_A2B10G10R10_UINT_PACK32;
            case DDSFile::DXGIFormat::R11G11B10_Float:
                return VK_FORMAT_B10G11R11_UFLOAT_PACK32;

            // 64-bit wide formats
            case DDSFile::DXGIFormat::R16G16B16A16_Float:
                return VK_FORMAT_R16G16B16A16_SFLOAT;
            case DDSFile::DXGIFormat::R16G16B16A16_SInt:
                return VK_FORMAT_R16G16B16A16_SINT;
            case DDSFile::DXGIFormat::R16G16B16A16_UInt:
                return VK_FORMAT_R16G16B16A16_UINT;
            case DDSFile::DXGIFormat::R16G16B16A16_UNorm:
                return VK_FORMAT_R16G16B16A16_UNORM;
            case DDSFile::DXGIFormat::R16G16B16A16_SNorm:
                return VK_FORMAT_R16G16B16A16_SNORM;

            case DDSFile::DXGIFormat::R32G32_Float:
                return VK_FORMAT_R32G32_SFLOAT;
            case DDSFile::DXGIFormat::R32G32_UInt:
                return VK_FORMAT_R32G32_UINT;
            case DDSFile::DXGIFormat::R32G32_SInt:
                return VK_FORMAT_R32G32_SINT;

            // 96-bit wide formats
            case DDSFile::DXGIFormat::R32G32B32_Float:
                return VK_FORMAT_R32G32B32_SFLOAT;
            case DDSFile::DXGIFormat::R32G32B32_UInt:
                return VK_FORMAT_R32G32B32_UINT;
            case DDSFile::DXGIFormat::R32G32B32_SInt:
                return VK_FORMAT_R32G32B32_SINT;

            // 128-bit wide formats
            case DDSFile::DXGIFormat::R32G32B32A32_Float:
                return VK_FORMAT_R32G32B32A32_SFLOAT;
            case DDSFile::DXGIFormat::R32G32B32A32_UInt:
                return VK_FORMAT_R32G32B32A32_UINT;
            case DDSFile::DXGIFormat::R32G32B32A32_SInt:
                return VK_FORMAT_R32G32B32A32_SINT;

            case DDSFile::DXGIFormat::R8G8_B8G8_UNorm:
            case DDSFile::DXGIFormat::G8R8_G8B8_UNorm:
            case DDSFile::DXGIFormat::YUY2:
            default:
                return VK_FORMAT_UNDEFINED;
        }
    }
}// namespace tinyddsloader

ImageIO::ImageDesc ImageIO::loadImage(const std::string& path) {
    auto ext = FileUtils::getFileExt(path);
    if (ext == "dds") {
        // LOGI("Loading DDS image from {}", path.c_str());
        ImageDesc desc;

        tinyddsloader::DDSFile dds;
        auto                   ret = dds.Load(path.c_str());
        if (tinyddsloader::Result::Success != ret) {
            LOGE("Failed to load dds file: {}", path.c_str());
        }

        int datasize = 0;
        desc.mipmaps.resize(dds.GetMipCount() * dds.GetArraySize());
        for (uint32_t arrayIdx = 0; arrayIdx < dds.GetArraySize(); arrayIdx++) {
            for (uint32_t mipIdx = 0; mipIdx < dds.GetMipCount(); mipIdx++) {
                const auto* imageData                                             = dds.GetImageData(mipIdx, arrayIdx);
                desc.mipmaps[arrayIdx * dds.GetMipCount() + mipIdx].level         = mipIdx;
                desc.mipmaps[arrayIdx * dds.GetMipCount() + mipIdx].extent.width  = imageData->m_width;
                desc.mipmaps[arrayIdx * dds.GetMipCount() + mipIdx].extent.height = imageData->m_height;
                desc.mipmaps[arrayIdx * dds.GetMipCount() + mipIdx].extent.depth  = imageData->m_depth;
                datasize += imageData->m_memSlicePitch;
            }
        }
        desc.data.resize(datasize);
        for (uint32_t arrayIdx = 0; arrayIdx < dds.GetArraySize(); arrayIdx++) {
            for (uint32_t mipIdx = 0; mipIdx < dds.GetMipCount(); mipIdx++) {
                const auto* imageData = dds.GetImageData(mipIdx, arrayIdx);
                memcpy(desc.data.data() + desc.mipmaps[arrayIdx * dds.GetMipCount() + mipIdx].offset, imageData->m_mem, imageData->m_memSlicePitch);
            }
        }
        desc.needGenerateMipmaps = dds.GetMipCount() == 1;
        desc.format              = ConvertFormatFromDxgiFormat(dds.GetFormat());
        desc.extent              = VkExtent3D{dds.GetWidth(), dds.GetHeight(), dds.GetDepth()};
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
    auto                 path_not_overwrite = FileUtils::getFilePath(path, "png", false);
    std::vector<uint8_t> image_data;
    image_data.resize(width * height * channels);
    memcpy(image_data.data(), data, image_data.size());
    std::ranges::transform(image_data.begin(), image_data.end(), image_data.begin(), [](uint8_t value) -> uint8_t {
        return static_cast<uint8_t>(std::pow(value / 255.0, 1.0 / 2.2) * 255);
    });
    LOGI("Saving image to {}", path_not_overwrite.c_str());
    stbi_write_png(path_not_overwrite.c_str(), width, height, channels, image_data.data(), width * channels);
}

void ImageIO::saveHdr(const std::string& path, void* data, int width, int height, int channels) {
   
}
