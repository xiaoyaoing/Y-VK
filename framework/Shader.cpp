#include "Shader.h"
#include <Device.h>

#include <fstream>

// static std::vector<char> readFile(const std::string &filename)
// {

//     file.close();
//     return buffer;
// }
Shader::Shader(Device &device, const std::string &path) {
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }
    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    // create shader
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = buffer.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(buffer.data());
    createInfo.pNext = nullptr;
    VK_CHECK_RESULT(vkCreateShaderModule(device.getHandle(), &createInfo, nullptr, &shader));
}

Shader::~Shader() {
}

VkPipelineShaderStageCreateInfo Shader::PipelineShaderStageCreateInfo() {
    VkPipelineShaderStageCreateInfo info{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    info.module = shader;
    info.pName = "main";
    return info;
}