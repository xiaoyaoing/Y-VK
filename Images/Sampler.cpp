//
// Created by 打工人 on 2023/3/6.
//

#include "Sampler.h"

Sampler::Sampler(ptr<Device> device, VkSamplerAddressMode sampleMode, VkFilter filter, float maxLod) {
    device = device;

    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType			 = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.pNext			 = nullptr;
    samplerInfo.addressModeU	 = sampleMode;
    samplerInfo.addressModeV	 = sampleMode;
    samplerInfo.addressModeW	 = sampleMode;
    samplerInfo.borderColor		 = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.compareOp		 = VK_COMPARE_OP_NEVER;
    samplerInfo.compareEnable	 = VK_FALSE;
    samplerInfo.minFilter		 = filter;
    samplerInfo.magFilter		 = filter;
    samplerInfo.minLod			 = 0.0f;
    samplerInfo.maxLod			 = maxLod;
    samplerInfo.mipmapMode		 = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias		 = 0.0;
    samplerInfo.maxAnisotropy	 = 1.0;

    ASSERT(vkCreateSampler(device->getHandle(), &samplerInfo, nullptr, &_sampler)==
            VK_SUCCESS,"create sampler")
}
