#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <memory>
#include <vector>
class ImageIO {
public:
    struct ImageDesc {
        std::vector<uint8_t> data;
        VkFormat             format;
        VkExtent3D           extent;
        bool                 needGenerateMipmaps{true};
    };
    static ImageDesc loadImage(const std::string& path);
    static void      saveImage(const std::string& path, std::shared_ptr<std::vector<uint8_t>>, int width, int height, int channels, bool ldr, bool hdr);
    static void      saveHdr(const std::string& path, void* data, int width, int height, int channels);
    static void      saveLdr(const std::string& path, void* data, int width, int height, int channels);
};
