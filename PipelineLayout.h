//
// Created by pc on 2023/8/11.
//
#pragma once

#include <Vulkan.h>

class Shader;

class Device;

class PipelineLayout {
public:
    PipelineLayout(Device &device, const std::vector<Shader *> &shader_modules);

    VkPipelineLayout layout;

};


