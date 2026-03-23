#pragma once
#include "Core/Buffer.h"
#include "Scene/Compoments/RenderPrimitive.h"

#include <memory>

class LoaderHelper {
public:
    std::unique_ptr<Buffer> LoadPrimitiveIdBuffer(CommandBuffer& commandBuffer,uint32_t primitiveCount);
    std::unique_ptr<Buffer> LoadPrimitiveUniformBuffers(CommandBuffer& commandBuffer,std::vector<PerPrimitiveUniform>& uniforms);
};
