#pragma once
#include <Vulkan.h>

class Sampler {
public:
    inline VkSampler getHandle() {
        return _sampler;
    }

protected:
    VkSampler  _sampler;
};
