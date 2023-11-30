#pragma once

#include <cstdint>
#include <string>
#include <vulkan/vulkan_core.h>

class SgImage;

struct BlockDim
{
    uint8_t x;
    uint8_t y;
    uint8_t z;
};


class AstcImageHelper
{
public:
    static void decodeAstcImage(SgImage& image);

    // static void loadImageResources(SgImage &image, const std::string &path);

private:
    static void decode(SgImage& image, BlockDim blockdim, VkExtent3D extent, const uint8_t* data);
};
