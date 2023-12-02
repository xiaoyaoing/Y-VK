#include "BlackBoard.h"
#include <RenderGraph/RenderGraph.h>

Blackboard::Blackboard(const RenderGraph& graph) noexcept : graph(graph)
{
}

Blackboard::~Blackboard() noexcept
{
}

RenderGraphHandle& Blackboard::operator[](std::string_view name) noexcept
{
    return mMap.at(name);
}

void Blackboard::put(std::string_view name, RenderGraphHandle handle) noexcept
{
    mMap.emplace(name, handle);
}

void Blackboard::remove(std::string_view name) noexcept
{
    mMap.erase(name);
}

RenderGraphHandle Blackboard::getHandle(std::string_view name) const noexcept
{
    return mMap.at(name);
}

const Image& Blackboard::getImage(std::string_view name) const noexcept
{
    return getHwImage(name).getVkImage();
}

const ImageView& Blackboard::getImageView(std::string_view name) const noexcept
{
    return getHwImage(name).getVkImageView();
}

const SgImage& Blackboard::getHwImage(std::string_view name) const noexcept
{
    return *graph.getResource(getHandle(name))->getHwTexture();
}
