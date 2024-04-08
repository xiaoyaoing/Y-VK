#include "PipelineLayout.h"
#include "Common/ResourceCache.h"
#include "Core/Device/Device.h"
#include "Core/Descriptor/DescriptorSet.h"

// PipelineLayout::PipelineLayout(Device& device, std::initializer_list<Shader> shaders): {
// }
PipelineLayout::PipelineLayout(Device& device, std::vector<Shader>& shaders_) : shaders(std::move(shaders_)), device(device) {
    create();
}

PipelineLayout::PipelineLayout(Device& device, const std::vector<std::string>& shaderPaths) : device(device) {
    shaders.reserve(shaderPaths.size());
    for (auto& shaderPath : shaderPaths) {
        shaders.emplace_back(device, FileUtils::getShaderPath(shaderPath));
    }

    create();
}

void PipelineLayout::create() {
    VkPipelineLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    for (auto& shader : shaders) {
        for (const auto& shaderResource : shader.getShaderResources()) {

            if (shaderResource.type == ShaderResourceType::Input ||
                shaderResource.type == ShaderResourceType::Output ||
                shaderResource.type == ShaderResourceType::PushConstant ||
                shaderResource.type == ShaderResourceType::SpecializationConstant) {
                continue;
            }

            shaderSets[shaderResource.set].push_back(shaderResource);
        }
    }

    //It is not necessary to fill empty descriptorSetLayouts. 2024-03-12

    // In some case,you may want to skip set1 to use set2,so some empty descriptorSetLayouts may be created.
    auto maxSet = std::ranges::max_element(shaderSets.begin(), shaderSets.end(), [](const auto& a, const auto& b) {
                      return a.first < b.first;
                  })->first;

    for (uint32_t i = 0; i < maxSet; i++)
        if (!shaderSets.contains(i))
            shaderSets[i] = {};

    std::vector<VkDescriptorSetLayout> vkDescriptorSetLayouts;

    for (const auto& shaderSet : shaderSets) {
        auto& descriptorSetLayout = device.getResourceCache().requestDescriptorLayout(shaderSet.first, shaderSet.second);

        descriptorLayouts[shaderSet.first] = &descriptorSetLayout;
        vkDescriptorSetLayouts.push_back(descriptorSetLayout.getHandle());
    }

    auto pushConstants = getShaderResources(ShaderResourceType::PushConstant);

    std::vector<VkPushConstantRange> pushConstantRanges(pushConstants.size());

    std::ranges::transform(pushConstants, pushConstantRanges.begin(), [](const auto& pushConstant) {
        return VkPushConstantRange{pushConstant.stages, pushConstant.offset, pushConstant.size};
    });

    createInfo.setLayoutCount         = toUint32(vkDescriptorSetLayouts.size());
    createInfo.pSetLayouts            = vkDescriptorSetLayouts.data();
    createInfo.pushConstantRangeCount = toUint32(pushConstantRanges.size());
    createInfo.pPushConstantRanges    = pushConstantRanges.data();

    VK_CHECK_RESULT(vkCreatePipelineLayout(device.getHandle(), &createInfo, nullptr, &layout))

    LOGI("Pipeline Layout created");
}

DescriptorLayout& PipelineLayout::getDescriptorSetLayout(std::size_t setIdx) {
    return *descriptorLayouts[setIdx];
}

VkPipelineLayout PipelineLayout::getHandle() const {
    return layout;
}

const std::vector<Shader>& PipelineLayout::getShaders() const {
    return shaders;
}

bool PipelineLayout::hasLayout(const uint32_t setIndex) const {
    return descriptorLayouts.contains(setIndex);
}

DescriptorLayout& PipelineLayout::getDescriptorLayout(const uint32_t setIndex) const {
    return *descriptorLayouts.at(setIndex);
}

const Shader& PipelineLayout::getShader(VkShaderStageFlagBits stage) const {
    for (const auto& shader : shaders) {
        if (shader.getStage() == stage) {
            return shader;
        }
    }
    spdlog::error("No such shader stage");
    return shaders[0];
}

const std::vector<ShaderResource> PipelineLayout::getShaderResources(const ShaderResourceType& type,
                                                                     VkShaderStageFlagBits     stage) const {
    std::vector<ShaderResource> found_resources;

    for (auto& shader : shaders) {
        if (shader.getStage() == stage || stage == VK_SHADER_STAGE_ALL) {
            for (const auto& shaderResource : shader.getShaderResources()) {
                if (shaderResource.type == type || type == ShaderResourceType::All)
                    found_resources.push_back(shaderResource);
            }
        }
    }

    return found_resources;
}

VkShaderStageFlags PipelineLayout::getPushConstantRangeStage(uint32_t size) const {
    VkShaderStageFlags stage{};
    for (auto& resource : getShaderResources(ShaderResourceType::PushConstant)) {
        if (resource.offset + resource.size <= size)
            stage |= resource.stages;
    }
    return stage;
}
const std::map<std::uint32_t, std::vector<ShaderResource>>& PipelineLayout::getShaderSets() const {
    return shaderSets;
}