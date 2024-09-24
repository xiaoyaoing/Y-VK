#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <memory>
#include <vector>


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


class ImageIO {
public:
    struct ImageDesc {
        std::vector<uint8_t> data;
        VkFormat             format;
        VkExtent3D           extent;
        std::vector<Mipmap>  mipmaps;
        bool                 needGenerateMipmaps{true};
    };
    static ImageDesc loadImage(const std::string& path);
    static void      saveImage(const std::string& path, std::shared_ptr<std::vector<uint8_t>>, int width, int height, int channels, bool ldr, bool hdr);
    static void      saveHdr(const std::string& path, void* data, int width, int height, int channels);
    static void      saveLdr(const std::string& path, void* data, int width, int height, int channels);
};
