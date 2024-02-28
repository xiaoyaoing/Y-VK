#pragma once

#include <string_view>
#include <RenderGraph/RenderGraphId.h>
#include <unordered_map>

class Buffer;
class Image;
class ImageView;
class SgImage;

class RenderGraph;

#define SWAPCHAIN_IMAGE_NAME "_swapchain_image"

class Blackboard {
    using Container = std::unordered_map<
        std::string_view,
        RenderGraphHandle>;

public:
    Blackboard(const RenderGraph& graph) noexcept;
    ~Blackboard() noexcept;

    RenderGraphHandle& operator[](std::string_view name) noexcept;
    void               put(std::string_view name, RenderGraphHandle handle) noexcept;

    RenderGraphHandle getHandle(std::string_view name) const noexcept;
    Image&            getImage(std::string_view name) const noexcept;
    const ImageView&  getImageView(std::string_view name) const noexcept;
    const SgImage&    getHwImage(std::string_view name) const noexcept;
    const Buffer&     getBuffer(std::string_view name) const noexcept;
    bool              contains(std::string_view name) const noexcept;

    void remove(std::string_view name) noexcept;

private:
    // RenderGraphHandle getHandle(std::string_view name) const noexcept;
    Container          mMap;
    const RenderGraph& graph;
};