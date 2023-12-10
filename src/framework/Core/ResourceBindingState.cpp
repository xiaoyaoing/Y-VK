//
// Created by pc on 2023/10/7.
//

#include "ResourceBindingState.h"

#include "Buffer.h"
#include "Images/Image.h"
#include "Images/ImageView.h"

void ResourceSet::bindBuffer(const Buffer& buffer, VkDeviceSize offset, VkDeviceSize range, uint32_t binding,
                             uint32_t array_element)
{
    resourceBindings[binding][array_element].dirty = true;
    resourceBindings[binding][array_element].buffer = &buffer;
    resourceBindings[binding][array_element].offset = offset;
    resourceBindings[binding][array_element].range = range;

    dirty = true;
}

void ResourceSet::bindImage(const ImageView& view, const Sampler& sampler, uint32_t binding, uint32_t array_element)
{
    resourceBindings[binding][array_element].dirty = true;
    resourceBindings[binding][array_element].image_view = &view;
    resourceBindings[binding][array_element].sampler = &sampler;
    resourceBindings[binding][array_element].layout = ImageUtil::getVkImageLayout(view.getImage().getLayout(view.getSubResourceRange()));
    dirty = true;
}

void ResourceSet::bindInput(const ImageView& view, uint32_t binding, uint32_t array_element)
{
    resourceBindings[binding][array_element].dirty = true;
    resourceBindings[binding][array_element].image_view = &view;
    resourceBindings[binding][array_element].layout = ImageUtil::getVkImageLayout(view.getImage().getLayout(view.getSubResourceRange()));
    dirty = true;
}

// void ResourceSet::bindAccel(const Accel& accel, uint32_t binding, uint32_t array_element)
// {
//     resourceBindings[binding][array_element].dirty = true;
//     resourceBindings[binding][array_element].accel = &accel;
//     dirty = true;
// }

// void ResourceSet::bindAccel1(const Accel& accel, uint32_t binding, uint32_t array_element)
// {
//     
// }


bool ResourceSet::isDirty() const
{
    return dirty;
}


const BindingMap<ResourceInfo>& ResourceSet::getResourceBindings() const
{
    return resourceBindings;
}




void ResourceBindingState::bindBuffer(uint32_t setId, const Buffer& buffer, VkDeviceSize offset, VkDeviceSize range,
                                      uint32_t binding, uint32_t array_element)
{
    resourceSets[setId].bindBuffer(buffer, offset, range, binding, array_element);
}

void ResourceBindingState::bindImage(uint32_t setId, const ImageView& view, const Sampler& sampler, uint32_t binding,
                                     uint32_t array_element)
{
    resourceSets[setId].bindImage(view, sampler, binding, array_element);
}

const std::unordered_map<uint32_t, ResourceSet>& ResourceBindingState::getResourceSets() const
{
    return resourceSets;
}
