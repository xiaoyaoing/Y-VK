#pragma once
#include "Vulkan.h"
#include <Device.h>
class Sampler {
public:
    inline VkSampler getHandle() {
        return _sampler;
    }
    Sampler(ptr<Device> device, VkSamplerAddressMode sampleMode, VkFilter filter, float maxLod);
protected:
    VkSampler  _sampler;
};
