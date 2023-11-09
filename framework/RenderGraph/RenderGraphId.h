#pragma once

#include <cstdint>
#include <limits>


class RenderGraphHandle
{
public:
    using Index = uint16_t;
    using Version = uint16_t;

    std::size_t getHash() const;

    // private:
    explicit RenderGraphHandle(Index index) noexcept: index(index)
    {
    }

    // index to the resource handle
    static constexpr uint16_t UNINITIALIZED = std::numeric_limits<Index>::max();
    Index index = UNINITIALIZED; // index to a ResourceSlot
    Version version = 0;

    // protected:
    // private ctor -- this cannot be constructed by users
    RenderGraphHandle() noexcept = default;

    // friend class RenderGraph;
    //
    // friend class Blackboard;

public:
    RenderGraphHandle(const RenderGraphHandle& rhs) noexcept = default;

    RenderGraphHandle& operator=(const RenderGraphHandle& rhs) noexcept = default;

    bool isInitialized() const noexcept { return index != UNINITIALIZED; }

    operator bool() const noexcept { return isInitialized(); }

    void clear() noexcept
    {
        index = UNINITIALIZED;
        version = 0;
    }

    bool operator<(const RenderGraphHandle& rhs) const noexcept
    {
        return index < rhs.index;
    }

    bool operator==(const RenderGraphHandle& rhs) const noexcept
    {
        return (index == rhs.index);
    }

    bool operator!=(const RenderGraphHandle& rhs) const noexcept
    {
        return !operator==(rhs);
    }
};


/** A typed handle on a resource */
template <typename RESOURCE>
class RenderGraphId : public RenderGraphHandle
{
public:
    using RenderGraphHandle::RenderGraphHandle;

    RenderGraphId() noexcept
    {
    }

    explicit RenderGraphId(RenderGraphHandle r) : RenderGraphHandle(r)
    {
    }
};

// namespace std
// {
//     template <typename T>
//     struct hash<RenderGraphId<T>>
//     {
//         std::size_t operator()(const RenderGraphId<T>& handle) const
//         {
//             return 0;
//         }
//     };
// }
