#include "HlslCompiler.h"

#include "Common/FIleUtils.h"

#include <wrl/client.h>
#include <d3d12shader.h>
#include <dxc/dxcapi.h>
#include <filesystem>
#include <fstream>

bool HlslCompiler::compileToSpirv(
    VkShaderStageFlagBits        stage,
    const std::vector<uint8_t>&  hlsl_source,
    const std::string&           entry_point,
    std::vector<std::uint32_t>&  spirv,
    std::string&                 info_log,
    const std::filesystem::path& shader_path,
    const ShaderKey&             shaderKey) {

    if (!InitializeDxcCompiler()) {
        info_log = "Failed to initialize DXC compiler";
        return false;
    }

    std::string profile = getTargetProfile(stage);
    if (profile.empty()) {
        info_log = "Unsupported shader stage";
        return false;
    }

    auto entryPointW   = StringToWString(entry_point);
    auto profileW      = StringToWString(profile);
    auto shaderFolderW = StringToWString(FileUtils::getShaderPath());

    std::vector<LPCWSTR> arguments = {
        L"-spirv",
        L"-fspv-target-env=vulkan1.2",
        L"-O3",
        L"-HV 2021",
        L"-E",
        entryPointW.c_str(),
        L"-T",
        profileW.c_str(),
    };

    arguments.push_back(L"-I");
    arguments.push_back(shaderFolderW.c_str());

    std::vector<std::wstring> define_strings;
    for (const auto& define : shaderKey.variant.get_processes()) {
        if (!define.empty() && define[0] == 'D') {
            std::wstring def = L"-D" + StringToWString(define.substr(1));
            define_strings.push_back(def);
            arguments.push_back(define_strings.back().c_str());
        }
    }

    Microsoft::WRL::ComPtr<IDxcBlobEncoding> source_blob;
    HRESULT                                  hr = dxc_utils->CreateBlob(
        hlsl_source.data(),
        static_cast<UINT32>(hlsl_source.size()),
        CP_UTF8,
        source_blob.GetAddressOf());

    if (FAILED(hr)) {
        info_log = "Failed to create source blob";
        return false;
    }

    DxcBuffer source_buffer;
    source_buffer.Ptr      = source_blob->GetBufferPointer();
    source_buffer.Size     = source_blob->GetBufferSize();
    source_buffer.Encoding = 0;

    Microsoft::WRL::ComPtr<IDxcResult> result;
    hr = dxc_compiler->Compile(
        &source_buffer,
        arguments.data(),
        arguments.size(),
        include_handler,
        IID_PPV_ARGS(result.GetAddressOf()));

    Microsoft::WRL::ComPtr<IDxcBlobUtf8> errors;
    result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(errors.GetAddressOf()), nullptr);
    if (FAILED(hr)) {
        if (errors && errors->GetStringLength() > 0) {
            info_log = std::string(errors->GetStringPointer());
        }
        return false;
    }

    Microsoft::WRL::ComPtr<IDxcBlob> spirv_blob;
    result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(spirv_blob.GetAddressOf()), nullptr);
    if (spirv_blob) {
        auto* data = static_cast<const uint32_t*>(spirv_blob->GetBufferPointer());
        auto  size = spirv_blob->GetBufferSize() / sizeof(uint32_t);
        spirv.assign(data, data + size);
    }

    return true;
}

bool HlslCompiler::InitializeDxcCompiler() {
    if (dxc_compiler && dxc_utils && include_handler) {
        return true;
    }

    HRESULT hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxc_compiler));
    if (FAILED(hr)) return false;

    hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxc_utils));
    if (FAILED(hr)) return false;

    hr = dxc_utils->CreateDefaultIncludeHandler(&include_handler);
    if (FAILED(hr)) return false;

    return true;
}

void HlslCompiler::CleanupDxcCompiler() {
    if (include_handler) {
        include_handler->Release();
        include_handler = nullptr;
    }
    if (dxc_utils) {
        dxc_utils->Release();
        dxc_utils = nullptr;
    }
    if (dxc_compiler) {
        dxc_compiler->Release();
        dxc_compiler = nullptr;
    }
}

void HlslCompiler::setTargetProfile(const std::string& profile) {
    target_profile = profile;
}

void HlslCompiler::setForceRecompile(bool _forceRecompile) {
    forceRecompile = _forceRecompile;
}

std::string HlslCompiler::getTargetProfile(VkShaderStageFlagBits stage) {
    switch (stage) {
        case VK_SHADER_STAGE_VERTEX_BIT:
            return "vs_" + target_profile;
        case VK_SHADER_STAGE_FRAGMENT_BIT:
            return "ps_" + target_profile;
        case VK_SHADER_STAGE_COMPUTE_BIT:
            return "cs_" + target_profile;
        default:
            return "";
    }
}

std::wstring HlslCompiler::StringToWString(const std::string& str) {
    if (str.empty()) return std::wstring();
    int          size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}
