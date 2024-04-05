#pragma once

#include <Scene/Scene.h>
#include <Scene/SceneLoader/SceneLoadingConfig.h>


class SceneLoaderInterface {
public:
    static std::unique_ptr<Scene> LoadSceneFromGLTFFile(Device& device, const std::string& path, const SceneLoadingConfig& config = {});
};
