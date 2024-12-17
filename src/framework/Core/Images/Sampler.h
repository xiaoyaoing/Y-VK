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

class SamplerManager {
public:
    static void Initialize(Device& device);
    
    // 常见的采样器配置
    static Sampler & GetRepeatLinearSampler(uint32_t mipLevels);
    static Sampler & GetClampToEdgeLinearSampler(uint32_t mipLevels);
    static Sampler & GetNearestSampler();

private:
    static Device* m_device; // 设备指针
};
