#pragma once

#include "Core/Shader/Shader.h"
#include "Core/Vulkan.h"
#include <optional>

class Device;

class DescriptorLayout {
public:
    DescriptorLayout(Device &device);

    // DescriptorLayout(DescriptorLayout & other) = delete;

    DescriptorLayout(Device &device, std::vector<Shader> &shaders);

    inline VkDescriptorSetLayout getHandle() const {
        return _layout;
    }


    void addBinding(VkShaderStageFlags stageFlags, uint32_t bindingPoint, uint32_t descCount, VkDescriptorType descType,
                    VkDescriptorBindingFlags bindingFlags);

    void createLayout(VkDescriptorSetLayoutCreateFlags flags);


    const VkDescriptorSetLayoutBinding &getLayoutBindingInfo(uint32_t bindingIndex) const;

    const VkDescriptorSetLayoutBinding &getLayoutBindingInfo(const std::string &name) const;

    bool hasLayoutBinding(const std::string &name) const;

    bool hasLayoutBinding(int bindingIndex) const;


    std::vector<VkDescriptorSetLayoutBinding> getBindings() const;

private:
    VkDescriptorSetLayout _layout;
    std::vector<std::pair<VkDescriptorSetLayoutBinding, VkDescriptorBindingFlags>> _descBindingInfos{};
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
    std::unordered_map<std::string, uint32_t> resourceLookUp;
    std::unordered_map<uint32_t, uint32_t> bindingLookUp;

    Device &_deivce;
};
