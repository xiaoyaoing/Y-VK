#include "BlackBoard.h"

Blackboard::Blackboard() noexcept {

}

Blackboard::~Blackboard() noexcept {

}

RenderGraphHandle &Blackboard::operator[](std::string_view name) noexcept {
    return mMap.at(name);
}

void Blackboard::put(std::string_view name, RenderGraphHandle handle) noexcept {
    mMap.emplace(name, handle);
}

void Blackboard::remove(std::string_view name) noexcept {
    mMap.erase(name);
}

RenderGraphHandle Blackboard::getHandle(std::string_view name) const noexcept {
    return mMap.at(name);
}
