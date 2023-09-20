#include "PipelineLayout.h"
#include "Common/ResourceCache.h"
#include "Device.h"
#include "Descriptor/DescriptorSet.h"


PipelineLayout::PipelineLayout(Device& device, std::vector<Shader>& shaders)
{
    VkPipelineLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    auto descriptorSetLayout = device.getResourceCache().requestDescriptorLayout(shaders);
    //   DescriptorSet descriptorSet;
}
