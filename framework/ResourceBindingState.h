//
// Created by pc on 2023/10/7.
//
#pragma  once

#include <unordered_map>
#include <Vulkan.h>

//template<class T>
//using BindingMap = std::unordered_map<

class Buffer;

class ImageView;

class Sampler;

struct ResourceInfo
{
    bool dirty{false};

    const Buffer* buffer{nullptr};

    VkDeviceSize offset{0};

    VkDeviceSize range{0};

    const ImageView* image_view{nullptr};

    const Sampler* sampler{nullptr};
};

struct ResourceSet
{
public:
    void
    bindBuffer(const Buffer& buffer, VkDeviceSize offset, VkDeviceSize range, uint32_t binding, uint32_t array_element);

    void bindImage(const ImageView& view, const Sampler& sampler, uint32_t binding, uint32_t array_element);

    const BindingMap<ResourceInfo>& getResourceBindings() const;

    bool isDirty() const;

private:
    BindingMap<ResourceInfo> resourceBindings;

    bool dirty{false};
};

class ResourceBindingState
{
public:
    void bindBuffer(uint32_t setId, const Buffer& buffer, VkDeviceSize offset, VkDeviceSize range, uint32_t binding,
                    uint32_t array_element);

    void
    bindImage(uint32_t setId, const ImageView& view, const Sampler& sampler, uint32_t binding, uint32_t array_element);


    const std::unordered_map<uint32_t, ResourceSet>& getResourceSets() const;

private:
    std::unordered_map<uint32_t, ResourceSet> resourceSets;
};
