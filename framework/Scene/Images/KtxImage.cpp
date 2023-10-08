////
//// Created by pc on 2023/8/28.
////
//#include <ktx.h>
//#include <ktxvulkan.h>
//#include "KtxImage.h"
//
//KtxImage::KtxImage(const std::string &path) {
//    ktxTexture *ktxTexture;
//    ktxResult result = ktxTexture_CreateFromNamedFile(path.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
//                                                      &ktxTexture);
//    assert(result == KTX_SUCCESS);
//    this->extent3D = VkExtent3D{ktxTexture->baseWidth, ktxTexture->baseHeight, 1};
//
//    ktx_uint8_t *ktxTextureData = ktxTexture_GetData(ktxTexture);
//    ktx_size_t ktxTextureSize = ktxTexture_GetSize(ktxTexture);
//
//    mipMaps.resize(ktxTexture->numLevels);
//    layers = ktxTexture->numLayers;
//    format = VK_FORMAT_R8G8B8A8_SRGB;
//    data = {ktxTextureData, ktxTextureData + ktxTextureSize};
//
//
//    if (layers > 1) {
//
//        for (uint32_t layer = 0; layer < layers; layer++) {
//            std::vector<VkDeviceSize> layerOffsets{};
//            for (uint32_t level = 0; level < mipMaps.size(); level++) {
//                ktx_size_t offset;
//
//                result = ktxTexture_GetImageOffset(ktxTexture, level, layer, 0, &offset);
//                assert(result == KTX_SUCCESS);
//                layerOffsets.push_back(static_cast<VkDeviceSize>(offset));
//            }
//            offsets.push_back(layerOffsets);
//        }
//    }
//}
