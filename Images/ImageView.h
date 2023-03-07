#pragma once
#include <Vulkan.h>

class ImageView {
public:
    inline VkImageView getHandle() {
        return _view;
    }

protected:
    VkImageView _view;
};
