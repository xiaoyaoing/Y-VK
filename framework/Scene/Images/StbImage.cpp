////
//// Created by pc on 2023/8/19.
////
//
//#include "StbImage.h"
//#include "stb_image.h"
//
//StbImage::StbImage(const std::string& path)
//{
//    int w, h, comp;
//    auto rawData = stbi_load(path.c_str(), &w, &h, &comp, 4);
//    comp = 4;
//    data = {rawData, rawData + w * h * comp};
//    stbi_image_free(rawData);
//
//    extent3D = {toUint32(w), toUint32(h), 1};
//    format = VK_FORMAT_R8G8B8A8_SRGB;
//}
