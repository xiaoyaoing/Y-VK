#pragma once

#include "Shader.h"
#include "Vulkan.h"

class Device;

class DescriptorLayout
{
public:
    DescriptorLayout(Device& device);

    // DescriptorLayout(DescriptorLayout & other) = delete;

    DescriptorLayout(Device& device, std::vector<Shader>& shaders);

    inline VkDescriptorSetLayout getHandle() const
    {
        return _layout;
    }


    void addBinding(VkShaderStageFlags stageFlags, uint32_t bindingPoint, uint32_t descCount, VkDescriptorType descType,
                    VkDescriptorBindingFlags bindingFlags);

    void createLayout(VkDescriptorSetLayoutCreateFlags flags);

    const VkDescriptorSetLayoutBinding& getLayoutBindingInfo(int bindingIndex) const;

    std::vector<VkDescriptorSetLayoutBinding> getBindings() const;

private:
    VkDescriptorSetLayout _layout;
    std::vector<std::pair<VkDescriptorSetLayoutBinding, VkDescriptorBindingFlags>> _descBindingInfos{};
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    Device& _deivce;
};
