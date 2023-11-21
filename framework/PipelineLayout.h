#pragma once

#include "Shader.h"

class DescriptorLayout;

class PipelineLayout
{
public:
    PipelineLayout(Device& device, std::vector<Shader>& shaders);

    DescriptorLayout& getDescriptorSetLayout(std::size_t setIdx);

    VkPipelineLayout getHandle() const;

    const std::vector<Shader>& getShaders() const;

    const Shader& getShader(VkShaderStageFlagBits stage) const;

    bool hasLayout(const uint32_t setIndex) const;

    const DescriptorLayout& getDescriptorLayout(const uint32_t setIndex) const;

    const std::vector<ShaderResource> getShaderResources(const ShaderResourceType& type = ShaderResourceType::All,
                                                         VkShaderStageFlagBits stage = VK_SHADER_STAGE_ALL) const;

    
    VkShaderStageFlags getPushConstantRangeStage(uint32_t size) const;
private:
    VkPipelineLayout layout;
    std::vector<DescriptorLayout*> descriptorLayouts;
    std::vector<Shader> shaders;
};
