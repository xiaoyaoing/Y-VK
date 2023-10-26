#include "PipelineLayout.h"
#include "Common/ResourceCache.h"
#include "Device.h"
#include "Descriptor/DescriptorSet.h"


PipelineLayout::PipelineLayout(Device& device, std::vector<Shader>& shaders): shaders(shaders)
{
    VkPipelineLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    auto& descriptorSetLayout = device.getResourceCache().requestDescriptorLayout(shaders);
    descriptorLayouts.push_back(&descriptorSetLayout);

    std::vector<VkDescriptorSetLayout> layouts;
    layouts.push_back(descriptorSetLayout.getHandle());
    createInfo.pSetLayouts = layouts.data();
    createInfo.setLayoutCount = layouts.size();

    VK_CHECK_RESULT(vkCreatePipelineLayout(device.getHandle(), &createInfo, nullptr, &layout))

    //   DescriptorSet descriptorSet;
}

DescriptorLayout& PipelineLayout::getDescriptorSetLayout(std::size_t setIdx)
{
    return *descriptorLayouts[setIdx];
}

VkPipelineLayout PipelineLayout::getHandle() const
{
    return layout;
}

const std::vector<Shader>& PipelineLayout::getShaders() const
{
    return shaders;
}

bool PipelineLayout::hasLayout(const uint32_t setIndex) const
{
    return setIndex < descriptorLayouts.size();
}

const DescriptorLayout& PipelineLayout::getDescriptorLayout(const uint32_t setIndex) const
{
    return *descriptorLayouts[setIndex];
}

const Shader& PipelineLayout::getShader(VkShaderStageFlagBits stage) const
{
    for (const auto& shader : shaders)
    {
        if (shader.getStage() == stage)
        {
            return shader;
        }
    }
    spdlog::error("No such shader stage");
    return shaders[0];
}

const std::vector<ShaderResource> PipelineLayout::getShaderResources(const ShaderResourceType& type,
                                                                     VkShaderStageFlagBits stage) const
{
    std::vector<ShaderResource> found_resources;

    for (auto& shader : shaders)
    {
        if (shader.getStage() == stage || stage == VK_SHADER_STAGE_ALL)
        {
            for (const auto& shaderResource : shader.getShaderResources())
            {
                if (shaderResource.type == type || type == ShaderResourceType::All)
                    found_resources.push_back(shaderResource);
            }
        }
    }

    return found_resources;
}
