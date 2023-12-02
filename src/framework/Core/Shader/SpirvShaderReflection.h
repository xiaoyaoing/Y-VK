#pragma once

#include "ext/spirv-cross/spirv_reflect.hpp"

#include "Shader.h"


class SpirvShaderReflection {
public:
    static bool reflectShaderResources(const std::vector<uint32_t> sprivSource, VkShaderStageFlagBits &  stage,
                                       std::vector<ShaderResource> &shaderResources);
};
