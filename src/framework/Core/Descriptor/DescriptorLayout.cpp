#include "DescriptorLayout.h"
#include "Core/Device/Device.h"

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
        case ShaderResourceType::AccelerationStructure:
            return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        default:
            throw std::runtime_error("No conversion possible for the shader resource type.");
            break;
    }
}

DescriptorLayout::DescriptorLayout(Device &device) : _deivce(device) {
}

DescriptorLayout::DescriptorLayout(Device &device, std::vector<Shader> &shaders) : _deivce(device) {
    VkDescriptorSetLayoutCreateInfo createInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    std::vector<VkDescriptorBindingFlags> bindingFlags;


    std::unordered_map<uint32_t,VkDescriptorSetLayoutBinding> bindingInfoMap{};
    
    for (auto &shader: shaders) {
        for (const auto &resource: shader.getShaderResources()) {
            // Skip shader resources whitout a binding point
            if (resource.type == ShaderResourceType::Input ||
                resource.type == ShaderResourceType::Output ||
                resource.type == ShaderResourceType::PushConstant ||
                resource.type == ShaderResourceType::SpecializationConstant) {
                continue;
            }
            auto binding = resource.binding;
            auto desc_type = find_descriptor_type(resource.type, false);
            if(bindingInfoMap.contains(binding))
            {
                CHECK_RESULT(bindingInfoMap[binding].descriptorType==desc_type)
                bindingInfoMap[binding].stageFlags |= resource.stages;
                continue;
            }
            VkDescriptorSetLayoutBinding bindingInfo{};
            bindingInfo.stageFlags = resource.stages;
            bindingInfo.binding = resource.binding;
            bindingInfo.descriptorCount = resource.arraySize;
            bindingInfo.descriptorType = find_descriptor_type(resource.type, false);

            bindingFlags.emplace_back(0);
            resourceLookUp.emplace(resource.name, resource.binding);
            //layoutBindings.emplace_back(bindingInfo);
            bindingInfoMap.emplace(binding,bindingInfo);
            bindingLookUp.emplace(bindingInfo.binding, bindingInfoMap.size()-1);
        }
    }

    for(auto & binding : bindingInfoMap)
    {
        layoutBindings.push_back(binding.second);
    }
    

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
    bindingLookUp.emplace(bindingPoint, _descBindingInfos.size() - 1);
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

const VkDescriptorSetLayoutBinding &DescriptorLayout::getLayoutBindingInfo(uint32_t bindingIndex) const {
    if (!bindingLookUp.contains(bindingIndex)) {
        spdlog::error("No such binding {}", bindingIndex);
    }
    return {layoutBindings[bindingLookUp.at(bindingIndex)]};
}

const VkDescriptorSetLayoutBinding &DescriptorLayout::getLayoutBindingInfo(const std::string &name) const {
    if (!resourceLookUp.contains(name)) {
        spdlog::error("No such binding {}", name);
    }
    return {getLayoutBindingInfo(resourceLookUp.at(name))};
}


std::vector<VkDescriptorSetLayoutBinding> DescriptorLayout::getBindings() const {
    return layoutBindings;
}

bool DescriptorLayout::hasLayoutBinding(const std::string &name) const {
    return resourceLookUp.contains(name);
}

bool DescriptorLayout::hasLayoutBinding(int bindingIndex) const {
    return bindingLookUp.contains(bindingIndex);
}
