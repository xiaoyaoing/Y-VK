#pragma once
#include "Shader.h"

class PipelineLayout
{
public:
    PipelineLayout(Device& device, std::vector<Shader>& shaders);
};
