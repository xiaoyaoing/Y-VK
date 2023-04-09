#pragma once
#include "Engine/Vulkan.h"

class Fence {
public:
    inline VkFence getHandle() {
        return _fence;
    }

protected:
    VkFence _fence;
};
