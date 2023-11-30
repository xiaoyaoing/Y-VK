//
// Created by pc on 2023/8/17.
//

#pragma once

#include "Core/Vulkan.h"

namespace GlslCompiler {
    bool compileToSpirv(VkShaderStageFlagBits stage, const std::vector<uint8_t> &glsl_source,
                        const std::string &entry_point,
                        std::vector<std::uint32_t> &spirv,
                        std::string &info_log);
};


