#pragma once

#include "Core/Vulkan.h"
#include "Core/Device/Device.h"

class Sampler {
public:
    inline VkSampler getHandle() const {
        return _sampler;
    }

    Sampler(Device& device, VkSamplerAddressMode sampleMode, VkFilter filter, float maxLod);
    ~Sampler();

protected:
    VkSampler _sampler;
    Device&   device;
};
