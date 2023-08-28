//
// Created by pc on 2023/8/12.
//
#include <Vulkan.h>
#include <Mesh.h>
#include "Device.h"

#pragma once

class Scene {
public:
    Scene(Device &device);
    
    std::vector<std::unique_ptr<Mesh>> meshes;
};


