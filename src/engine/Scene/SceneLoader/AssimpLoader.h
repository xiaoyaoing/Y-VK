#pragma once

#include "SceneLoadingConfig.h"
#include <Scene/Scene.h>
class AssimpLoader {
public:
    static std::unique_ptr<Scene> LoadSceneFromGLTFFile(Device& device, const std::string& path, const SceneLoadingConfig & config = {});

};
