#pragma once

#include <RenderGraph/RenderGraphId.h>
#include <unordered_map>
#include <string>

class Buffer;
class Image;
class ImageView;
class SgImage;
    
class RenderGraph;

static const std::string RENDER_VIEW_PORT_IMAGE_NAME = "_RENDER_VIEW_PORT_IMAGE_NAME_";
static const std::string RT_IMAGE_NAME =  "_RT_IMAGE_NAME_";
static const std::string DEPTH_IMAGE_NAME = "_DEPTH_IMAGE_NAME_";

class Blackboard {
    using Container = std::unordered_map<
        std::string,
        RenderGraphHandle>;

public:
    Blackboard(const RenderGraph& graph) noexcept;
    ~Blackboard() noexcept;

    RenderGraphHandle& operator[](const std::string& name) noexcept;
    void               put(const std::string& name, RenderGraphHandle handle) noexcept;

    RenderGraphHandle getHandle(const std::string& name) const noexcept;
    Image&            getImage(const std::string& name) const noexcept;
    const ImageView&  getImageView(const std::string& name) const noexcept;
    const SgImage&    getHwImage(const std::string& name) const noexcept;
    const Buffer&     getBuffer(const std::string& name) const noexcept;
    bool              contains(const std::string& name) const noexcept;

    void remove(const std::string& name) noexcept;

private:
    // RenderGraphHandle getHandle(const std::string & name) const noexcept;
    Container          mMap;
    const RenderGraph& graph;
};