#pragma once

#include "Vulkan.h"
#include "Device.h"

class Sampler
{
public:
    inline VkSampler getHandle() const
    {
        return _sampler;
    }

    Sampler(Device& device, VkSamplerAddressMode sampleMode, VkFilter filter, float maxLod);

protected:
    VkSampler _sampler;
};
