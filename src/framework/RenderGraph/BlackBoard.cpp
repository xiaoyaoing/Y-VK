#include "BlackBoard.h"
#include <RenderGraph/RenderGraph.h>

Blackboard::Blackboard(const RenderGraph& graph) noexcept : graph(graph) {
}

Blackboard::~Blackboard() noexcept {
}

RenderGraphHandle& Blackboard::operator[](const std::string& name) noexcept {
    return mMap.at(name);
}

void Blackboard::put(const std::string& name, RenderGraphHandle handle) noexcept {
    mMap.emplace(name, handle);
}

void Blackboard::remove(const std::string& name) noexcept {
    mMap.erase(name);
}

RenderGraphHandle Blackboard::getHandle(const std::string& name) const noexcept {
    if(!mMap.contains(name))
        LOGE("Blackboard does not contain resource: {}", name);
    return mMap.at(name);
}

Image& Blackboard::getImage(const std::string& name) const noexcept {
    return getHwImage(name).getVkImage();
}

const ImageView& Blackboard::getImageView(const std::string& name) const noexcept {
    return getHwImage(name).getVkImageView();
}

const SgImage& Blackboard::getHwImage(const std::string& name) const noexcept {
    return *graph.getTexture(getHandle(name))->getHwTexture();
}

const Buffer& Blackboard::getBuffer(const std::string& name) const noexcept {
    return *graph.getBuffer(getHandle(name))->getHwBuffer();
}

bool Blackboard::contains(const std::string& name) const noexcept {
    return mMap.contains(name);
}