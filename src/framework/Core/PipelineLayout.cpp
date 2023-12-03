#include "PipelineLayout.h"
#include "Common/ResourceCache.h"
#include "Core/Device/Device.h"
#include "Core/Descriptor/DescriptorSet.h"


PipelineLayout::PipelineLayout(Device& device, std::vector<Shader>& shaders_) : shaders(std::move(shaders_))
{
    VkPipelineLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    auto& descriptorSetLayout = device.getResourceCache().requestDescriptorLayout(shaders);
    descriptorLayouts.push_back(&descriptorSetLayout);

    auto pushConstants = getShaderResources(ShaderResourceType::PushConstant);

    std::vector<VkPushConstantRange> pushConstantRanges(pushConstants.size());

    std::ranges::transform(pushConstants, pushConstantRanges.begin(), [](const auto& pushConstant)
    {
        return VkPushConstantRange{pushConstant.stages, pushConstant.offset, pushConstant.size};
    });

    std::vector<VkDescriptorSetLayout> layouts;
    layouts.push_back(descriptorSetLayout.getHandle());
    createInfo.pSetLayouts = layouts.data();
    createInfo.setLayoutCount = toUint32(layouts.size());
    createInfo.pushConstantRangeCount = pushConstantRanges.size();
    createInfo.pPushConstantRanges = pushConstantRanges.data();

    VK_CHECK_RESULT(vkCreatePipelineLayout(device.getHandle(), &createInfo, nullptr, &layout))
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

VkShaderStageFlags PipelineLayout::getPushConstantRangeStage(uint32_t size) const
{
    VkShaderStageFlags stage{};
    for (auto& resource : getShaderResources(ShaderResourceType::PushConstant))
    {
        if (resource.offset + resource.size <= size)
            stage |= resource.stages;
    }
    return stage;
}
