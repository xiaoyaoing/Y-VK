#pragma once
#include "Core/Buffer.h"

struct Accel
{
    VkAccelerationStructureKHR accel = VK_NULL_HANDLE;
    std::unique_ptr<Buffer> buffer;
};
