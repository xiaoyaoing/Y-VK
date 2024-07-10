#pragma once

class Buffer;
class SgImage;
namespace TextureHelper {
    void           Initialize();
    const SgImage* GetLogo();
    const SgImage* GetBlueNoise();
    Buffer * GetBlueNoiseBuffer();

}// namespace TextureHelper
