#include "DescriptorLayout.h"
#include "Engine/Device.h"
DescriptorLayout::DescriptorLayout(ptr<Device> device) :_deivce(device) {

}

void DescriptorLayout::addBinding(VkShaderStageFlags stageFlags,
                                     uint32_t bindingPoint,
                                     uint32_t descCount,
                                     VkDescriptorType descType,
                                     VkDescriptorBindingFlags bindingFlags)
{
    VkDescriptorSetLayoutBinding bindingInfo = {};
    bindingInfo.stageFlags			= stageFlags;
    bindingInfo.binding				= bindingPoint;
    bindingInfo.descriptorCount		= descCount;
    bindingInfo.descriptorType		= descType;
    bindingInfo.pImmutableSamplers	= nullptr;

    _descBindingInfos.emplace_back(std::move(bindingInfo), bindingFlags);
}

void DescriptorLayout::createLayout(VkDescriptorSetLayoutCreateFlags flags) {
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
    std::vector<VkDescriptorBindingFlags> bindingFlags;
    layoutBindings.reserve(_descBindingInfos.size());
    bindingFlags.reserve(_descBindingInfos.size());

    for (const auto& info : _descBindingInfos)
    {
        layoutBindings.emplace_back(info.first);
        bindingFlags.emplace_back(info.second);
    }
    _descBindingInfos.clear();
    _descBindingInfos.shrink_to_fit();

    VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo = {};
    flagsInfo.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    flagsInfo.pNext			= nullptr;
    flagsInfo.bindingCount	= static_cast<uint32_t>(bindingFlags.size());
    flagsInfo.pBindingFlags = bindingFlags.data();

    VkDescriptorSetLayoutCreateInfo descSetLayoutInfo = {};
    descSetLayoutInfo.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descSetLayoutInfo.bindingCount	= static_cast<uint32_t>(layoutBindings.size());
    descSetLayoutInfo.pBindings		= layoutBindings.data();
    descSetLayoutInfo.pNext			= &flagsInfo;
    descSetLayoutInfo.flags			= flags;
    ASSERT(vkCreateDescriptorSetLayout(_deivce->getHandle(),&descSetLayoutInfo, nullptr,&_layout)==VK_SUCCESS,"create descriptor layout");
}
