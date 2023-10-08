#include "PipelineLayout.h"
#include "Common/ResourceCache.h"
#include "Device.h"
#include "Descriptor/DescriptorSet.h"


PipelineLayout::PipelineLayout(Device &device, std::vector<Shader> &shaders) {
    VkPipelineLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    auto &descriptorSetLayout = device.getResourceCache().requestDescriptorLayout(shaders);
    descriptorLayouts.push_back(&descriptorSetLayout);

    std::vector<VkDescriptorSetLayout> layouts;
    layouts.push_back(descriptorSetLayout.getHandle());
    createInfo.pSetLayouts = layouts.data();
    createInfo.setLayoutCount = layouts.size();

    VK_CHECK_RESULT(vkCreatePipelineLayout(device.getHandle(), &createInfo, nullptr, &layout))

    //   DescriptorSet descriptorSet;
}

DescriptorLayout &PipelineLayout::getDescriptorSetLayout(std::size_t setIdx) {
    return *descriptorLayouts[setIdx];
}

VkPipelineLayout PipelineLayout::getHandle() const {
    return layout;
}

const std::vector<Shader> &PipelineLayout::getShaders() const {
    return shaders;
}

bool PipelineLayout::hasLayout(const uint32_t setIndex) const {
    return setIndex < descriptorLayouts.size();
}

const DescriptorLayout &PipelineLayout::getDescriptorLayout(const uint32_t setIndex) const {
    return *descriptorLayouts[setIndex];
}
