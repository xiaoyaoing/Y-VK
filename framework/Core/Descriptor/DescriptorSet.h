#include "Core/Vulkan.h"

#pragma once

class Device;

class DescriptorPool;

class DescriptorLayout;

class Buffer;

class DescriptorSet {

public:
    VkDescriptorSet &getHandle() {
        return _descriptorSet;
    }

    DescriptorSet(Device &device,
                  DescriptorPool &descriptorPool,
                  DescriptorLayout &descriptorSetLayout,
                  const uint32_t descSetCount);

    DescriptorSet(Device &device,
                  const DescriptorLayout &descriptorSetLayout,
                  DescriptorPool &descriptorPool,
                  const BindingMap<VkDescriptorBufferInfo> &bufferInfos = {},
                  const BindingMap<VkDescriptorImageInfo> &imageInfos = {});

    void updateBuffer(const std::vector<Buffer *> &buffers,
                      const uint32_t dstBinding,
                      const uint32_t descCount,
                      VkDescriptorType descType);

    void
    updateImage(const std::vector<VkDescriptorImageInfo> &imageInfos, const uint32_t dstBinding,
                VkDescriptorType descType);


    ~DescriptorSet();

private:
    VkDescriptorSet _descriptorSet;
    Device &_device;

    BindingMap<VkDescriptorBufferInfo> bufferInfos;
    BindingMap<VkDescriptorImageInfo> imageInfos;
};