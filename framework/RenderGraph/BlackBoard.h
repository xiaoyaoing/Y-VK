#pragma once

#include <RenderGraph/RenderGraphId.h>
#include <unordered_map>


class BlackBoard
{
    class Blackboard {
        using Container = std::unordered_map<
                std::string_view,
                RenderGraphHandle>;

    public:
        Blackboard() noexcept;
        ~Blackboard() noexcept;

        RenderGraphHandle& operator [](std::string_view name) noexcept;

        void put(std::string_view name, RenderGraphHandle handle) noexcept;

        template<typename T>
        RenderGraphId<T> get(std::string_view&& name) const noexcept {
            return static_cast<RenderGraphId<T>>(getHandle(std::forward<std::string_view>(name)));
        }

        void remove(std::string_view name) noexcept;

    private:
        RenderGraphHandle getHandle(std::string_view name) const noexcept;
        Container mMap;
};
