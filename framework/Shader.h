#include "Vulkan.h"

#pragma once

class Device;


class Shader {


public:
    enum SHADER_LOAD_MODE {
        SPV,
        ORIGIN_SHADER
    };
    
    Shader(Device &device, const std::string &path, SHADER_LOAD_MODE mode = SPV);

    ~Shader();

    VkPipelineShaderStageCreateInfo PipelineShaderStageCreateInfo(VkShaderStageFlagBits shaderStage);

private:
    bool initFromSpv();

    bool initFromOriginShader();

    VkShaderModule shader;
    VkShaderStageFlagBits tage;

};
