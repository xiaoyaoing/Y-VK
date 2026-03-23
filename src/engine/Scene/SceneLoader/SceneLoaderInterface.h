#pragma once

#include "ObjLoader.hpp"

#include <Scene/Scene.h>
#include <Scene/SceneLoader/SceneLoadingConfig.h>
#include "Scene/RuntimeSceneManager.h"

class SceneLoaderInterface {
public:
    static std::unique_ptr<Scene> LoadSceneFromFile(Device& device, const std::string& path, const SceneLoadingConfig & config = {});
    static std::unique_ptr<Primitive> loadSpecifyTypePrimitive(Device& device, const std::string& type);
    static std::vector<std::unique_ptr<Primitive>> loadSpecifyTypePrimitives(Device& device, const std::vector<std::string> & type);
    static std::unique_ptr<Primitive> loadPrimitiveFromPritiveData(Device& device, PrimitiveData* primitiveData);

};
