//
// Created by pc on 2023/8/17.
//

#pragma once

#include "Shader.h"
#include "Core/Vulkan.h"
#include <glslang/Public/ShaderLang.h>

class ShaderVarint;
class GlslCompiler {
public:
    static bool        compileToSpirv(VkShaderStageFlagBits stage, const std::vector<uint8_t>& glsl_source, const std::string& entry_point, std::vector<std::uint32_t>& spirv, std::string& info_log, const std::filesystem::path& shader_path,const ShaderKey & shaderKey);
    static void        setEnvTarget(glslang::EShTargetLanguage        target_language,
                                    glslang::EShTargetLanguageVersion target_language_version);
    static void setForceRecompile(bool _forceRecompile);
    inline static bool forceRecompile{};

private:
    static glslang::EShTargetLanguage        env_target_language;
    static glslang::EShTargetLanguageVersion env_target_language_version;
};
