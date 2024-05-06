#include "ImageIO.h"
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "ctpl_stl.h"
#include "Common/Log.h"

#include <stb_image_write.h>
#include <vector>
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
