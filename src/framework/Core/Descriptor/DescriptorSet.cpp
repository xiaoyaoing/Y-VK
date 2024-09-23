#include "DescriptorSet.h"
#include "DescriptorLayout.h"
#include "DescriptorPool.h"
#include "Common/ResourceCache.h"
#include "Core/Buffer.h"
#include "Core/ResourceCachingHelper.h"
#include "Core/Device/Device.h"

DescriptorSet::~DescriptorSet() {
    if (_descriptorSet != VK_NULL_HANDLE) {
        //  vkFreeDescriptorSets(_device.getHandle(), , 1, &_descriptorSet);
    }
}

// template<class T>
// std::vector<T>

DescriptorSet::DescriptorSet(Device& device, const DescriptorLayout& descriptorSetLayout, DescriptorPool& descriptorPool, const BindingMap<VkDescriptorBufferInfo>& bufferInfos_, const BindingMap<VkDescriptorImageInfo>& imageInfos_, const BindingMap<VkWriteDescriptorSetAccelerationStructureKHR>& accelInfos_)
    : _device(device), _descriptorSet(descriptorPool.allocate()), bufferInfos(bufferInfos_), imageInfos(imageInfos_), accelerations(accelInfos_), descriptorSetLayout(&descriptorSetLayout), descriptorPool(&descriptorPool) {
    std::string buffer_info;
    for (auto& bufferIt : bufferInfos) {
        buffer_info += "Binding: " + std::to_string(bufferIt.first) + " ";
        for (auto& elementIt : bufferIt.second) {
            buffer_info += "Element: " + std::to_string(elementIt.first) + " " + std::to_string(reinterpret_cast<unsigned long long>(elementIt.second.buffer)) + " " + std::to_string(elementIt.second.offset) + " " + std::to_string(elementIt.second.range) + " ";
        }
    }

    for (auto& bufferIt : bufferInfos) {
        auto  bindingIndex   = bufferIt.first;
        auto& bufferBindings = bufferIt.second;

        auto& bindingInfo = descriptorSetLayout.getLayoutBindingInfo(bindingIndex);

        for (auto& elementIt : bufferBindings) {
            auto& bufferInfo = elementIt.second;

            VkWriteDescriptorSet writeSet{
                .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet          = _descriptorSet,
                .dstBinding      = bindingIndex,
                .dstArrayElement = elementIt.first,
                .descriptorCount = 1,
                .descriptorType  = bindingInfo.descriptorType,
                .pBufferInfo     = &bufferInfo,
            };

            writeSets.push_back(writeSet);
        }
    }

    for (auto& imageIt : imageInfos) {
        auto  bindingIndex  = imageIt.first;
        auto& imageBindings = imageIt.second;

        auto& bindingInfo = descriptorSetLayout.getLayoutBindingInfo(bindingIndex);

        // std::unordered_map<VkDescriptorType,>
        for (auto& elementIt : imageBindings) {
            auto& imageInfo = elementIt.second;

            VkWriteDescriptorSet writeSet{
                .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet          = _descriptorSet,
                .dstBinding      = bindingIndex,
                .dstArrayElement = elementIt.first,
                .descriptorCount = 1,
                .descriptorType  = bindingInfo.descriptorType,
                .pImageInfo      = &imageInfo,
            };

            writeSets.push_back(writeSet);
        }
    }

    for (auto& accelIt : accelerations) {
        auto  bindingIndex  = accelIt.first;
        auto& accelBindings = accelIt.second;

        auto& bindingInfo = descriptorSetLayout.getLayoutBindingInfo(bindingIndex);

        for (auto& elementIt : accelBindings) {
            auto& accelInfo = elementIt.second;

            VkWriteDescriptorSet writeSet{
                .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext           = &accelInfo,
                .dstSet          = _descriptorSet,
                .dstBinding      = bindingIndex,
                .dstArrayElement = elementIt.first,
                .descriptorCount = 1,
                .descriptorType  = bindingInfo.descriptorType,
            };

            writeSets.push_back(writeSet);
        }
    }
}


DescriptorSet::DescriptorSet(DescriptorSet&& descriptorSet) : _device(descriptorSet._device), _descriptorSet(descriptorSet._descriptorSet), writeSets(std::move(descriptorSet.writeSets)), mUpdatedBindings(std::move(descriptorSet.mUpdatedBindings)),
                                                              bufferInfos(std::move(descriptorSet.bufferInfos)), imageInfos(std::move(descriptorSet.imageInfos)), accelerations(std::move(descriptorSet.accelerations)), descriptorSetLayout(descriptorSet.descriptorSetLayout), descriptorPool(descriptorSet.descriptorPool) {
    descriptorSet._descriptorSet = VK_NULL_HANDLE;
}
bool DescriptorSet::operator==(const DescriptorSet& desc) const {
    if (imageInfos.size() != desc.imageInfos.size() || bufferInfos.size() != desc.bufferInfos.size() || accelerations.size() != desc.accelerations.size()) {
        return false;
    }

    for (auto& bufferIt : bufferInfos) {
        auto descBufferIt = desc.bufferInfos.find(bufferIt.first);
        if (descBufferIt == desc.bufferInfos.end()) {
            return false;
        }
        if (bufferIt.second.size() != descBufferIt->second.size()) {
            return false;
        }
        for (auto& elementIt : bufferIt.second) {
            auto descElementIt = descBufferIt->second.find(elementIt.first);
            if (descElementIt == descBufferIt->second.end()) {
                return false;
            }
            if (elementIt.second.buffer != descElementIt->second.buffer || elementIt.second.offset != descElementIt->second.offset || elementIt.second.range != descElementIt->second.range) {
                return false;
            }
        }
    }

    for (auto& imageIt : imageInfos) {
        auto descImageIt = desc.imageInfos.find(imageIt.first);
        if (descImageIt == desc.imageInfos.end()) {
            return false;
        }
        if (imageIt.second.size() != descImageIt->second.size()) {
            return false;
        }
        for (auto& elementIt : imageIt.second) {
            auto descElementIt = descImageIt->second.find(elementIt.first);
            if (descElementIt == descImageIt->second.end()) {
                return false;
            }
            if (elementIt.second.imageView != descElementIt->second.imageView || elementIt.second.imageLayout != descElementIt->second.imageLayout || elementIt.second.sampler != descElementIt->second.sampler) {
                return false;
            }
        }
    }
    return true;
}

void DescriptorSet::update(const std::vector<uint32_t>& bindings_to_update) {
    // vkUpdateDescriptorSets(_device.getHandle(), 1, &writeSet, 0, nullptr);
    std::vector<VkWriteDescriptorSet> write_operations;
    std::vector<size_t>               write_operation_hashes;
    std::vector<size_t>               write_pos_hashes;
    // LOGI("mUpdatedBindings 1 {}", mUpdatedBindings.size())
    // If the 'bindings_to_update' vector is empty, we want to write to all the bindings
    // (but skipping all to-update bindings that haven't been written yet)
    if (bindings_to_update.empty()) {
        for (size_t i = 0; i < writeSets.size(); i++) {
            const auto& write_operation = writeSets[i];

            size_t write_operation_hash = 0;
            hash_param(write_operation_hash, write_operation);

            size_t write_pos_hash = 0;
            hash_param(write_pos_hash, write_operation.dstBinding, write_operation.dstArrayElement);

            auto update_pair_it = mUpdatedBindings.find(write_pos_hash);
            if (update_pair_it == mUpdatedBindings.end() || update_pair_it->second != write_operation_hash) {
                write_operations.push_back(write_operation);
                write_operation_hashes.push_back(write_operation_hash);
                write_pos_hashes.push_back(write_pos_hash);
            }
        }
    } else {
        // Otherwise we want to update the binding indices present in the 'bindings_to_update' vector.
        // (again, skipping those to update but not updated yet)
        for (size_t i = 0; i < writeSets.size(); i++) {
            const auto& write_operation = writeSets[i];

            if (std::find(bindings_to_update.begin(), bindings_to_update.end(), write_operation.dstBinding) != bindings_to_update.end()) {
                size_t write_operation_hash = 0;
                hash_param(write_operation_hash, write_operation);

                auto update_pair_it = mUpdatedBindings.find(write_operation.dstBinding);
                if (update_pair_it == mUpdatedBindings.end() || update_pair_it->second != write_operation_hash) {
                    write_operations.push_back(write_operation);
                    write_operation_hashes.push_back(write_operation_hash);
                }
            }
        }
    }

    // LOGI("DescriptorSet::update {}", reinterpret_cast<size_t>(this->_descriptorSet));

    // Perform the Vulkan call to update the DescriptorSet by executing the write operations
    if (!write_operations.empty()) {
        vkUpdateDescriptorSets(_device.getHandle(),
                               toUint32(write_operations.size()),
                               write_operations.data(),
                               0,
                               nullptr);
    }

    // Store the bindings from the write operations that were executed by vkUpdateDescriptorSets (and their hash)
    // to prevent overwriting by future calls to "update()"
    for (size_t i = 0; i < write_operations.size(); i++) {
        mUpdatedBindings[write_pos_hashes[i]] = write_operation_hashes[i];
    }
    // LOGI("mUpdatedBindings 2 {}", mUpdatedBindings.size())
}
