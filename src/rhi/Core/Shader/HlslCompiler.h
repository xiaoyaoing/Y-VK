#ifndef HLSLCOMPILER_H
#define HLSLCOMPILER_H

#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Unknwn.h>
#include "dxcapi.h"
#include "Shader.h"
#include <filesystem>
#include <vector>
#include <string>

class ShaderVarint;
class HlslCompiler {
public:
    static bool compileToSpirv(
        VkShaderStageFlagBits stage,
        const std::vector<uint8_t>& hlsl_source,
        const std::string& entry_point,
        std::vector<std::uint32_t>& spirv,
        std::string& info_log,
        const std::filesystem::path& shader_path,
        const ShaderKey& shaderKey
    );

    static void setTargetProfile(const std::string& target_profile);
    
    static void setForceRecompile(bool _forceRecompile);
    inline static bool forceRecompile{};
    HlslCompiler();
    ~HlslCompiler();
private:
    static std::string getTargetProfile(VkShaderStageFlagBits stage);
    
    static bool InitializeDxcCompiler();
    
    static void CleanupDxcCompiler();

    inline static IDxcCompiler3* dxc_compiler{nullptr};
    inline static IDxcUtils* dxc_utils{nullptr};
    inline static IDxcIncludeHandler* include_handler{nullptr};

    inline static std::string target_profile{"6_0"}; 
    static std::wstring StringToWString(const std::string& str);
};

#endif