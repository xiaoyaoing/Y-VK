#include "Vulkan.h"

#pragma once

class Device;
class Shader
{
public:
    Shader(Device &device, const std::string &path);
    ~Shader();
    VkPipelineShaderStageCreateInfo PipelineShaderStageCreateInfo();

private:
    VkShaderModule shader;
    VkShaderStageFlagBits tage;

};
