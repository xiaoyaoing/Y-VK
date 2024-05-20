#pragma once

#include "Core/Vulkan.h"
#include "Core/Device/Device.h"

class Sampler {
public:
    inline VkSampler getHandle() const {
        return _sampler;
    }

    Sampler(Device& device, VkSamplerAddressMode sampleMode, VkFilter filter, float maxLod,VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT, VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT, VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT);
    ~Sampler();
    Sampler(Sampler&&);

protected:
    VkSampler _sampler;
    Device&   device;
};
