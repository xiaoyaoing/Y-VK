//
// Created by pc on 2023/8/12.
//

#include "Scene.h"
#include "Device.h"
#include <Mesh.h>

Scene::Scene(Device &device) {
    meshes.push_back(std::make_unique<Mesh>("E:\\code\\VulkanTutorial\\resources\\viking_room.obj"));
    meshes[0]->createBuffer(device);
}