#include "Shader.h"
#include "Core/Device/Device.h"
#include <filesystem>

#include <fstream>
#include "Common/FIleUtils.h"
#include "Core/Shader/GlslCompiler.h"

#include "spdlog/fmt/bundled/os.h"
#include "Core/Shader/SpirvShaderReflection.h"

Shader::SHADER_LOAD_MODE getShaderMode(const std::string& ext) {
    if (ext == "spv")
        return Shader::SPV;
    return Shader::ORIGIN_SHADER;
}

size_t Shader::getId() const {
    return id;
}

VkShaderStageFlagBits Shader::getStage() const {
    return stage;
}

VkShaderStageFlagBits find_shader_stage(const std::string& ext) {
    if (ext == "vert") {
        return VK_SHADER_STAGE_VERTEX_BIT;
    } else if (ext == "frag") {
        return VK_SHADER_STAGE_FRAGMENT_BIT;
    } else if (ext == "comp") {
        return VK_SHADER_STAGE_COMPUTE_BIT;
    } else if (ext == "geom") {
        return VK_SHADER_STAGE_GEOMETRY_BIT;
    } else if (ext == "tesc") {
        return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    } else if (ext == "tese") {
        return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    } else if (ext == "rgen") {
        return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    } else if (ext == "rahit") {
        return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
    } else if (ext == "rchit") {
        return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    } else if (ext == "rmiss") {
        return VK_SHADER_STAGE_MISS_BIT_KHR;
    } else if (ext == "rint") {
        return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
    } else if (ext == "rcall") {
        return VK_SHADER_STAGE_CALLABLE_BIT_KHR;
    } else if (ext == "mesh") {
        return VK_SHADER_STAGE_MESH_BIT_EXT;
    } else if (ext == "task") {
        return VK_SHADER_STAGE_TASK_BIT_EXT;
    }

    throw std::runtime_error("File extension `" + ext + "` does not have a vulkan shader stage.");

}

static  std::string SPV_CACHED_PATH = "spvcachedFiles/";

static std::string GetSpvPathFromShaderPath(const std::string& path) {
    static std::string shaderFolder = "shaders/";
    size_t idx =  path.find(shaderFolder);
    if (idx == std::string::npos) {
        LOGE("Failed to find shaders folder in shader path: {}", path.c_str());
    }
    idx+= shaderFolder.size();
    return   path.substr(0, idx) + SPV_CACHED_PATH + path.substr(idx) + ".spv";
    
}

static std::ofstream OpenOrCreateFile(const std::string& path) {
    std::filesystem::path folder = std::filesystem::path(path).parent_path();
    while(true) {
        if(std::filesystem::exists(folder)) {
            break;
        }
        std::filesystem::create_directory(folder);
        folder = folder.parent_path();
    }
    return std::ofstream(path, std::ios::binary);
}
Shader::Shader(Device& device, std::string path) : device(device) {
    auto                  mode = getShaderMode(FileUtils::getFileExt(path));
    std::vector<uint32_t> spirvCode;

    std::string originShaderPath = path;
    std::string         spvPath = GetSpvPathFromShaderPath(path);

    stage                        = find_shader_stage(FileUtils::getFileExt(originShaderPath));
    if (path.ends_with(".spv")) {
        mode = SPV;
    } else {
        if (std::filesystem::exists(spvPath)) {
            auto spvUpdateTime    = std::filesystem::last_write_time(spvPath);
            auto shaderUpdateTime = std::filesystem::last_write_time(path);
            // localtime_s(spvUpdateTime);
            if (spvUpdateTime > shaderUpdateTime && !GlslCompiler::forceRecompile) {
                mode = SPV;
                LOGI("Cached shader spv found: {},Spv last update time {}", path, FileUtils::getFileTimeStr(spvPath));
            }
        }
    }

    if (mode == SHADER_LOAD_MODE::SPV) {
        path = spvPath;
        std::ifstream file(path, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            LOGE("Failed  to open file", path.c_str());
        }
        size_t            fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        auto shaderSource = FileUtils::readShaderBinary(path);

        if (shaderSource.empty()) {
            mode = SHADER_LOAD_MODE::ORIGIN_SHADER;
        }

        spirvCode.assign(reinterpret_cast<const uint32_t*>(shaderSource.data()),
                         reinterpret_cast<const uint32_t*>(shaderSource.data() + shaderSource.size()));
    }
    if (mode == SHADER_LOAD_MODE::ORIGIN_SHADER) {
        path = originShaderPath;
        std::string shaderLog;
        auto        shaderBuffer = FileUtils::readShaderBinary(path);
        if (!GlslCompiler::compileToSpirv(stage, shaderBuffer, "main", spirvCode, shaderLog, path)) {
            LOGE("Failed to compile shader {}, Error: {}", path, shaderLog.c_str())
        } else {
            LOGI("Shader compiled from source code succrssfully", path.c_str());
        }

        std::ofstream file = OpenOrCreateFile(spvPath);
        file.write(reinterpret_cast<const char*>(spirvCode.data()), spirvCode.size() * sizeof(uint32_t));
        file.close();
    }
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = spirvCode.size() * sizeof(uint32_t);
    createInfo.pCode    = reinterpret_cast<const uint32_t*>(spirvCode.data());
    createInfo.pNext    = nullptr;
    VK_CHECK_RESULT(vkCreateShaderModule(device.getHandle(), &createInfo, nullptr, &shader))

    std::hash<std::string> hasher{};
    id = hasher(std::string(reinterpret_cast<const char*>(spirvCode.data()),
                            reinterpret_cast<const char*>(spirvCode.data() + spirvCode.size())));

    if (!SpirvShaderReflection::reflectShaderResources(spirvCode, stage, resources)) {
        LOGE("Failed to reflect shader resources,Shader path: {}", path.c_str())
    }
}
// Shader::Shader(const ShaderSource& source, const ShaderVarint& varint) {
//
//     //GlslCompiler::compileToSpirv()
//
//
//     // VkShaderModuleCreateInfo createInfo{};
//     // createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
//     // createInfo.codeSize = spirvCode.size() * sizeof(uint32_t);
//     // createInfo.pCode    = reinterpret_cast<const uint32_t*>(spirvCode.data());
//     // createInfo.pNext    = nullptr;
//     // VK_CHECK_RESULT(vkCreateShaderModule(device.getHandle(), &createInfo, nullptr, &shader))
//     //
//     // std::hash<std::string> hasher{};
//     // id = hasher(std::string(reinterpret_cast<const char*>(spirvCode.data()),
//     //                         reinterpret_cast<const char*>(spirvCode.data() + spirvCode.size())));
//     //
//     // if (!SpirvShaderReflection::reflectShaderResources(spirvCode, stage, resources)) {
//     //     LOGE("Failed to reflect shader resources,Shader path: {}", path.c_str())
//     // }
// }

Shader::~Shader() {
    // vkDestroyShaderModule(device.getHandle(), shader, nullptr);
}

VkPipelineShaderStageCreateInfo Shader::PipelineShaderStageCreateInfo() const {
    VkPipelineShaderStageCreateInfo info{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    info.stage  = stage;
    info.module = shader;
    info.pName  = "main";
    return info;
}