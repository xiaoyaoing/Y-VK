#include "DescriptorLayout.h"
#include "Device.h"

inline VkDescriptorType find_descriptor_type(ShaderResourceType resourceType, bool dynamic) {
    switch (resourceType) {
        case ShaderResourceType::InputAttachment:
            return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
            break;
        case ShaderResourceType::Image:
            return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            break;
        case ShaderResourceType::ImageSampler:
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            break;
        case ShaderResourceType::ImageStorage:
            return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            break;
        case ShaderResourceType::Sampler:
            return VK_DESCRIPTOR_TYPE_SAMPLER;
            break;
        case ShaderResourceType::BufferUniform:
            if (dynamic) {
                return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            } else {
                return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            }
            break;
        case ShaderResourceType::BufferStorage:
            if (dynamic) {
                return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
            } else {
                return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            }
            break;
        default:
            throw std::runtime_error("No conversion possible for the shader resource type.");
            break;
    }
}

DescriptorLayout::DescriptorLayout(Device &device) : _deivce(device) {
}

DescriptorLayout::DescriptorLayout(Device &device, std::vector<Shader> &shaders) : _deivce(device) {
    VkDescriptorSetLayoutCreateInfo createInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
    std::vector<VkDescriptorBindingFlags> bindingFlags;

    for (auto &shader: shaders) {
        for (const auto &resource: shader.getShaderResources()) {
            // Skip shader resources whitout a binding point
            if (resource.type == ShaderResourceType::Input ||
                resource.type == ShaderResourceType::Output ||
                resource.type == ShaderResourceType::PushConstant ||
                resource.type == ShaderResourceType::SpecializationConstant) {
                continue;
            }
            VkDescriptorSetLayoutBinding bindingInfo{};
            bindingInfo.stageFlags = resource.stages;
            bindingInfo.binding = resource.binding;
            bindingInfo.descriptorCount = resource.arraySize;
            bindingInfo.descriptorType = find_descriptor_type(resource.type, false);

            bindingFlags.emplace_back(0);
            layoutBindings.emplace_back(bindingInfo);
            resourceLookUp.emplace(resource.name, resource.binding);
        }
    }


    bindings = layoutBindings;
    VkDescriptorSetLayoutCreateInfo descSetLayoutInfo = {};
    descSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descSetLayoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
    descSetLayoutInfo.pBindings = layoutBindings.data();
    descSetLayoutInfo.flags = 0;
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(_deivce.getHandle(), &descSetLayoutInfo, nullptr, &_layout))
}

void DescriptorLayout::addBinding(VkShaderStageFlags stageFlags,
                                  uint32_t bindingPoint,
                                  uint32_t descCount,
                                  VkDescriptorType descType,
                                  VkDescriptorBindingFlags bindingFlags) {
    VkDescriptorSetLayoutBinding bindingInfo = {};
    bindingInfo.stageFlags = stageFlags;
    bindingInfo.binding = bindingPoint;
    bindingInfo.descriptorCount = descCount;
    bindingInfo.descriptorType = descType;
    bindingInfo.pImmutableSamplers = nullptr;

    _descBindingInfos.emplace_back(std::move(bindingInfo), bindingFlags);
}

void DescriptorLayout::createLayout(VkDescriptorSetLayoutCreateFlags flags) {
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
    std::vector<VkDescriptorBindingFlags> bindingFlags;
    layoutBindings.reserve(_descBindingInfos.size());
    bindingFlags.reserve(_descBindingInfos.size());

    for (const auto &info: _descBindingInfos) {
        layoutBindings.emplace_back(info.first);
        bindingFlags.emplace_back(info.second);
    }
    _descBindingInfos.clear();
    _descBindingInfos.shrink_to_fit();

    VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo = {};
    flagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    flagsInfo.pNext = nullptr;
    flagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());
    flagsInfo.pBindingFlags = bindingFlags.data();

    VkDescriptorSetLayoutCreateInfo descSetLayoutInfo = {};
    descSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descSetLayoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
    descSetLayoutInfo.pBindings = layoutBindings.data();
    // descSetLayoutInfo.pNext = &flagsInfo;
    descSetLayoutInfo.flags = flags;
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(_deivce.getHandle(), &descSetLayoutInfo, nullptr, &_layout));
}

const VkDescriptorSetLayoutBinding &DescriptorLayout::getLayoutBindingInfo(int bindingIndex) const {
    return bindings[bindingIndex];
}

const VkDescriptorSetLayoutBinding &DescriptorLayout::getLayoutBindingInfo(std::string &name) const {
    if (resourceLookUp.contains(name)) {
        spdlog::error("No such binding {}", name);

    }
    return {getLayoutBindingInfo(resourceLookUp.at(name))};
}


std::vector<VkDescriptorSetLayoutBinding> DescriptorLayout::getBindings() const {
    return bindings;
}

bool DescriptorLayout::hasLayoutBinding(std::string &name) const {
    return resourceLookUp.contains(name);
}
