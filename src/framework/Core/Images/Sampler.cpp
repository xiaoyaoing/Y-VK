//
// Created by 打工人 on 2023/3/6.
//

#include "Sampler.h"
#include "Common/ResourceCache.h"   
Sampler::Sampler(Device& device, VkSamplerAddressMode sampleMode, VkFilter filter, float maxLod,VkSamplerAddressMode addressModeU,VkSamplerAddressMode addressModeV,VkSamplerAddressMode addressModeW) : device(device) {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(device.getPhysicalDevice(), &properties);
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter               = filter;
    samplerInfo.minFilter               = filter;
    samplerInfo.addressModeU            = addressModeU;
    samplerInfo.addressModeV            = addressModeV;
    samplerInfo.addressModeW            = addressModeW;
    samplerInfo.anisotropyEnable        = VK_FALSE;
    samplerInfo.maxAnisotropy           = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable           = VK_FALSE;
    samplerInfo.compareOp               = VK_COMPARE_OP_NEVER;
    samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod                  = 0.0f;
    samplerInfo.maxLod                  = maxLod;
    //    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    //    samplerInfo.mipLodBias = 0.0;
    //    samplerInfo.maxAnisotropy = 1.0;

    VK_CHECK_RESULT(vkCreateSampler(device.getHandle(), &samplerInfo, nullptr, &_sampler));
}
Sampler::~Sampler() {
    if (_sampler != VK_NULL_HANDLE) {
        vkDestroySampler(device.getHandle(), _sampler, nullptr);
    }
}
Sampler::Sampler(Sampler&& other) : device(other.device), _sampler(other._sampler) {
    other._sampler = VK_NULL_HANDLE;
}

Device* SamplerManager::m_device = nullptr;

void SamplerManager::Initialize(Device& device) {
    m_device = &device;
}

Sampler & SamplerManager::GetRepeatLinearSampler(uint32_t mipLevels) {
    return m_device->getResourceCache().requestSampler(VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_FILTER_LINEAR, mipLevels);
}

Sampler & SamplerManager::GetClampToEdgeLinearSampler(uint32_t mipLevels) {
    return m_device->getResourceCache().requestSampler(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_FILTER_LINEAR, mipLevels);
}

Sampler & SamplerManager::GetNearestSampler() {
    return m_device->getResourceCache().requestSampler(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_FILTER_NEAREST, 1);
}