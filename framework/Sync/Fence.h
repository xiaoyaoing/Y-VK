#pragma once
#include "Vulkan.h"

class Fence {
public:
    inline VkFence getHandle() {
        return _fence;
    }

protected:
    VkFence _fence;
};
