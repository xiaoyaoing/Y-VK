//
// Created by pc on 2023/8/19.
//

#include "StbImage.h"
#include "stb_image.h"

sg::StbImage::StbImage(const std::string &path) {
    int w, h, comp;
    auto rawData = stbi_load(path.c_str(), &w, &h, &comp, 4);
    comp = 4;
//    data = std::vector<uint8_t>(w * h * 4);
//    for (int i = 0; i < w * h * 4; i++) {
//        data[i] = rawData[i % (3 * w * h)];
//    }
    data = {rawData, rawData + w * h * comp};
    stbi_image_free(rawData);

    extent3D = {toUint32(w), toUint32(h), 1};
    format = VK_FORMAT_R8G8B8A8_SRGB;

}
