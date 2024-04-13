#pragma once

#include "Core/Shader/Shader.h"

class DescriptorLayout;

class PipelineLayout {
public:
    PipelineLayout(Device& device, std::vector<Shader>& shaders);
    PipelineLayout(Device& device, const std::vector<std::string>& shaderPaths);

    DescriptorLayout& getDescriptorSetLayout(std::size_t setIdx);

    VkPipelineLayout getHandle() const;

    const std::vector<Shader>& getShaders() const;

    const Shader& getShader(VkShaderStageFlagBits stage) const;

    bool hasLayout(const uint32_t setIndex) const;

    DescriptorLayout& getDescriptorLayout(const uint32_t setIndex) const;

    const std::vector<ShaderResource> getShaderResources(const ShaderResourceType& type  = ShaderResourceType::All,
                                                         VkShaderStageFlagBits     stage = VK_SHADER_STAGE_ALL) const;

    VkShaderStageFlags getPushConstantRangeStage(uint32_t size) const;

    const std::map<std::uint32_t, std::vector<ShaderResource>>& getShaderSets() const;

private:
    void create();

    VkPipelineLayout                                     layout;
    Device&                                              device;
    std::map<uint32_t, DescriptorLayout*>                descriptorLayouts;
    std::vector<Shader>                                  shaders;
    std::map<std::uint32_t, std::vector<ShaderResource>> shaderSets;
};