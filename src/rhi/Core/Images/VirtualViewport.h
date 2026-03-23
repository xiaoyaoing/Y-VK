#pragma once
#include "Scene/SgImage.h"

#include <vector>

class VirtualViewport {
public:
    VirtualViewport(Device & device,VkExtent2D extent, uint32_t imageCount);
    SgImage & getImage(uint32_t index);
    VkExtent2D getExtent();
protected:
    std::vector<std::unique_ptr<SgImage>> images;

};
