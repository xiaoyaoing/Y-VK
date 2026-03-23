#include "Core/Vulkan.h"

#pragma once

class Device;

class DescriptorPool;

class DescriptorLayout;

class Buffer;

class DescriptorSet {

public:
    VkDescriptorSet getHandle() const {
        return _descriptorSet;
    }

    DescriptorSet(Device&                                                         device,
                  const DescriptorLayout&                                         descriptorSetLayout,
                  DescriptorPool&                                                 descriptorPool,
                  const BindingMap<VkDescriptorBufferInfo>&                       bufferInfos_   = {},
                  const BindingMap<VkDescriptorImageInfo>&                        imageInfos_    = {},
                  const BindingMap<VkWriteDescriptorSetAccelerationStructureKHR>& accelerations_ = {});

    DescriptorSet(const DescriptorSet&) = delete;
    DescriptorSet(DescriptorSet&&);

    bool operator==(const DescriptorSet&) const;
    void update(const std::vector<uint32_t>& bindings_to_update);

    ~DescriptorSet();

private:
    VkDescriptorSet                    _descriptorSet;
    Device&                            _device;
    std::vector<VkWriteDescriptorSet>  writeSets;
    std::unordered_map<size_t, size_t> mUpdatedBindings;

    BindingMap<VkDescriptorBufferInfo>                       bufferInfos;
    BindingMap<VkDescriptorImageInfo>                        imageInfos;
    BindingMap<VkWriteDescriptorSetAccelerationStructureKHR> accelerations;
    const DescriptorLayout*                                  descriptorSetLayout;
    const DescriptorPool*                                    descriptorPool;
};