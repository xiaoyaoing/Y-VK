#pragma once

#include <Engine/Vulkan.h>
#include "Engine/CommandBuffer.h"

class RenderFrame {
public:
    VkSemaphore requestSemOwnerShip() {
        return nullptr;
    }

    CommandBuffer &requestCommandBuffer(){
        return buffer;
    }
protected:
    CommandBuffer buffer;
};
