#include "Shader.h"
#include "Core/Device/Device.h"
#include <filesystem>

#include <fstream>
#include "Common/FIleUtils.h"
#include "Core/Shader/GlslCompiler.h"

#include "spdlog/fmt/bundled/os.h"
#include "Core/Shader/SpirvShaderReflection.h"

#include <stack>

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
Shader Shader::operator=(const Shader& other) {
    return *this;
}
// Shader& Shader::operator=(const Shader& other) {
//     return *this;
// }

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

static std::string SPV_CACHED_PATH = "spvcachedFiles/";

static std::string GetSpvPathFromShaderPath(const std::string& path, const std::string& shaderDefinitions) {
    static std::string shaderFolder = "shaders/";
    size_t             idx          = path.find(shaderFolder);
    if (idx == std::string::npos) {
        LOGE("Failed to find shaders folder in shader path: {}", path.c_str());
    }
    idx += shaderFolder.size();
    return path.substr(0, idx) + SPV_CACHED_PATH + path.substr(idx) + shaderDefinitions + ".spv";
}

static std::ofstream OpenOrCreateFile(const std::string& path) {
    std::filesystem::path             folder = std::filesystem::path(path).parent_path();
    std::stack<std::filesystem::path> folderToCreate;
    while (true) {
        if (std::filesystem::exists(folder)) {
            break;
        }
        folderToCreate.push(folder);
        folder = folder.parent_path();
    }
    while (!folderToCreate.empty()) {
        std::filesystem::create_directory(folderToCreate.top());
        folderToCreate.pop();
    }
    return std::ofstream(path, std::ios::binary);
}

ShaderVariant::ShaderVariant(std::string&& preamble, std::vector<std::string>&& processes) : preamble{std::move(preamble)},
                                                                                             processes{std::move(processes)} {
    update_id();
}

size_t ShaderVariant::get_id() const {
    return id;
}

void ShaderVariant::add_definitions(const std::vector<std::string>& definitions) {
    for (auto& definition : definitions) {
        add_define(definition);
    }
}

void ShaderVariant::add_define(const std::string& def) {
    processes.push_back("D" + def);

    std::string tmp_def = def;

    // The "=" needs to turn into a space
    size_t pos_equal = tmp_def.find_first_of("=");
    if (pos_equal != std::string::npos) {
        tmp_def[pos_equal] = ' ';
    }

    preamble.append("#define " + tmp_def + "\n");

    update_id();
}

void ShaderVariant::add_undefine(const std::string& undef) {
    processes.push_back("U" + undef);

    preamble.append("#undef " + undef + "\n");

    update_id();
}

void ShaderVariant::add_runtime_array_size(const std::string& runtime_array_name, size_t size) {
    if (runtime_array_sizes.find(runtime_array_name) == runtime_array_sizes.end()) {
        runtime_array_sizes.insert({runtime_array_name, size});
    } else {
        runtime_array_sizes[runtime_array_name] = size;
    }
}

void ShaderVariant::set_runtime_array_sizes(const std::unordered_map<std::string, size_t>& sizes) {
    this->runtime_array_sizes = sizes;
}

const std::string& ShaderVariant::get_preamble() const {
    return preamble;
}

const std::vector<std::string>& ShaderVariant::get_processes() const {
    return processes;
}

const std::unordered_map<std::string, size_t>& ShaderVariant::get_runtime_array_sizes() const {
    return runtime_array_sizes;
}

void ShaderVariant::clear() {
    preamble.clear();
    processes.clear();
    runtime_array_sizes.clear();
    update_id();
}

void ShaderVariant::update_id() {
    std::hash<std::string> hasher{};
    id = hasher(preamble);
}

Shader::Shader(Device& device, const ShaderKey& key, VkShaderStageFlagBits spv_stage) : device(device) {
    std::string           shaderFilePath = FileUtils::getShaderPath(key.path);
    auto                  mode           = getShaderMode(FileUtils::getFileExt(shaderFilePath));
    std::vector<uint32_t> spirvCode;

    auto defineString = key.variant.get_preamble();

    if (shaderFilePath.ends_with(".spv")) {
        mode  = SPV;
        stage = spv_stage;
    } else {
        stage               = find_shader_stage(FileUtils::getFileExt(shaderFilePath));
        std::string spvPath = GetSpvPathFromShaderPath(shaderFilePath, defineString);
        if (std::filesystem::exists(spvPath)) {
            auto spvUpdateTime    = std::filesystem::last_write_time(spvPath);
            auto shaderUpdateTime = std::filesystem::last_write_time(shaderFilePath);
            // localtime_s(spvUpdateTime);
            if (spvUpdateTime > shaderUpdateTime && !GlslCompiler::forceRecompile) {
                mode           = SPV;
                shaderFilePath = spvPath;
                LOGI("Cached shader spv found: {},Spv last update time {}", shaderFilePath, FileUtils::getFileTimeStr(spvPath));
            }
        }
    }

    if (mode == SHADER_LOAD_MODE::SPV) {
        auto shaderSource = FileUtils::readShaderBinary(shaderFilePath);
        if (shaderSource.empty()) {
            mode = SHADER_LOAD_MODE::ORIGIN_SHADER;
        }
        spirvCode.assign(reinterpret_cast<const uint32_t*>(shaderSource.data()),
                         reinterpret_cast<const uint32_t*>(shaderSource.data() + shaderSource.size()));
    }
    if (mode == SHADER_LOAD_MODE::ORIGIN_SHADER) {
        std::string shaderLog;
        auto        shaderBuffer = FileUtils::readShaderBinary(shaderFilePath);
        if (!GlslCompiler::compileToSpirv(stage, shaderBuffer, "main", spirvCode, shaderLog, shaderFilePath, key)) {
            LOGE("Failed to compile shader {}, Error: {}", shaderFilePath, shaderLog.c_str())
        } else {
            LOGI("Shader compiled from source code succrssfully {}", shaderFilePath.c_str());
        }
        std::string   spvPath = GetSpvPathFromShaderPath(shaderFilePath, defineString);
        std::ofstream file    = OpenOrCreateFile(spvPath);
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
        LOGE("Failed to reflect shader resources,Shader path: {}", shaderFilePath.c_str())
    }
}

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