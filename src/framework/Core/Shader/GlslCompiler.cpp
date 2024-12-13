#include "GlslCompiler.h"
#include "Shader.h"
#include <filesystem>
#include <SPIRV/GlslangToSpv.h>
#include <StandAlone/DirStackFileIncluder.h>
#include <glslang/Public/ResourceLimits.h>

inline EShLanguage FindShaderLanguage(VkShaderStageFlagBits stage) {
    switch (stage) {
        case VK_SHADER_STAGE_VERTEX_BIT:
            return EShLangVertex;

        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
            return EShLangTessControl;

        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
            return EShLangTessEvaluation;

        case VK_SHADER_STAGE_GEOMETRY_BIT:
            return EShLangGeometry;

        case VK_SHADER_STAGE_FRAGMENT_BIT:
            return EShLangFragment;

        case VK_SHADER_STAGE_COMPUTE_BIT:
            return EShLangCompute;

        case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
            return EShLangRayGen;

        case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
            return EShLangAnyHit;

        case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
            return EShLangClosestHit;

        case VK_SHADER_STAGE_MISS_BIT_KHR:
            return EShLangMiss;

        case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:
            return EShLangIntersect;

        case VK_SHADER_STAGE_CALLABLE_BIT_KHR:
            return EShLangCallable;

        case VK_SHADER_STAGE_MESH_BIT_EXT:
            return EShLangMesh;

        case VK_SHADER_STAGE_TASK_BIT_EXT:
            return EShLangTask;

        default:
            return EShLangVertex;
    }
}

glslang::EShTargetLanguage        GlslCompiler::env_target_language         = glslang::EShTargetSpv;
glslang::EShTargetLanguageVersion GlslCompiler::env_target_language_version = glslang::EShTargetLanguageVersion::EShTargetSpv_1_5;

bool GlslCompiler::compileToSpirv(VkShaderStageFlagBits stage, const std::vector<uint8_t>& glsl_source, const std::string& entry_point, std::vector<std::uint32_t>& spirv, std::string& info_log, const std::filesystem::path& shader_path, const ShaderKey& shaderKey) {
    //return false;
    // Initialize glslang library.
    glslang::InitializeProcess();

    auto messages = static_cast<EShMessages>(EShMsgDefault | EShMsgVulkanRules | EShMsgSpvRules);

    EShLanguage language = FindShaderLanguage(stage);
    auto        source   = std::string(glsl_source.begin(), glsl_source.end());

    const char* file_name_list[1] = {""};
    auto        shader_source     = reinterpret_cast<const char*>(source.data());

    glslang::TShader shader(language);

    shader.setStringsWithLengthsAndNames(&shader_source, nullptr, file_name_list, 1);
    shader.setEntryPoint(entry_point.c_str());
    shader.setSourceEntryPoint(entry_point.c_str());

    // if (!definitions.empty()) {
    shader.setPreamble(shaderKey.variant.get_preamble().c_str());
    // }
    shader.addProcesses(shaderKey.variant.get_processes());
    if (GlslCompiler::env_target_language != glslang::EShTargetLanguage::EShTargetNone)
        shader.setEnvTarget(GlslCompiler::env_target_language, GlslCompiler::env_target_language_version);

    DirStackFileIncluder includeDir;
    includeDir.pushExternalLocalDirectory(shader_path.parent_path().string());

    if (!shader.parse(GetDefaultResources(), 460, false, messages, includeDir)) {
        info_log = std::string(shader.getInfoLog()) + "\n" + std::string(shader.getInfoDebugLog());
        return false;
    }

    // Add shader to new program object.
    glslang::TProgram program;
    program.addShader(&shader);

    // Link program.
    if (!program.link(messages)) {
        info_log = std::string(program.getInfoLog()) + "\n" + std::string(program.getInfoDebugLog());
        return false;
    }

    // Save any info log that was generated.
    if (shader.getInfoLog()) {
        info_log += std::string(shader.getInfoLog()) + "\n" + std::string(shader.getInfoDebugLog()) + "\n";
    }

    if (program.getInfoLog()) {
        info_log += std::string(program.getInfoLog()) + "\n" + std::string(program.getInfoDebugLog());
    }

    glslang::TIntermediate* intermediate = program.getIntermediate(language);

    // Translate to SPIRV.
    if (!intermediate) {
        info_log += "Failed to get shared intermediate code.\n";
        return false;
    }

    spv::SpvBuildLogger logger;

    glslang::GlslangToSpv(*intermediate, spirv, &logger);

    info_log += logger.getAllMessages() + "\n";

    // Shutdown glslang library.
    glslang::FinalizeProcess();

    return true;
}

void GlslCompiler::setEnvTarget(glslang::EShTargetLanguage        target_language,
                                glslang::EShTargetLanguageVersion target_language_version) {
    GlslCompiler::env_target_language         = target_language;
    GlslCompiler::env_target_language_version = target_language_version;
}
void GlslCompiler::setForceRecompile(bool _forceRecompile) {
    forceRecompile = _forceRecompile;
}