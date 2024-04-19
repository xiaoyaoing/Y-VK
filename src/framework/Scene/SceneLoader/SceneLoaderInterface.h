#pragma once

#include <Scene/Scene.h>
#include <Scene/SceneLoader/SceneLoadingConfig.h>


class SceneLoaderInterface {
public:
    static std::unique_ptr<Scene> LoadSceneFromFile(Device& device, const std::string& path, const SceneLoadingConfig & config = {});
    static std::unique_ptr<Primitive> loadSpecifyTypePrimitive(Device& device, const std::string& type);
};
