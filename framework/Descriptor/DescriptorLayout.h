#include "Vulkan.h"

class Device;

class DescriptorLayout {
public:
    DescriptorLayout(Device &device);

    inline VkDescriptorSetLayout getHandle() {
        return _layout;
    }

    void addBinding(VkShaderStageFlags stageFlags, uint32_t bindingPoint, uint32_t descCount, VkDescriptorType descType,
                    VkDescriptorBindingFlags bindingFlags);

    void createLayout(VkDescriptorSetLayoutCreateFlags flags);

private:
    VkDescriptorSetLayout _layout;
    std::vector<std::pair<VkDescriptorSetLayoutBinding, VkDescriptorBindingFlags>> _descBindingInfos{};
    Device &_deivce;

};