#include "SceneLoaderInterface.h"

#include "gltfloader.h"
#include "jsonloader.h"


static std::unordered_map<std::string,std::function<std::unique_ptr<Scene>(Device& device, const std::string& path, const SceneLoadingConfig & config)>> sceneLoaders = {
    {
       "json",Jsonloader::LoadSceneFromGLTFFile
    }, {"gltf",GltfLoading::LoadSceneFromGLTFFile}
};
std::unique_ptr<Scene> SceneLoaderInterface::LoadSceneFromFile(Device& device, const std::string& path, const SceneLoadingConfig & config) {
    std::string extension = path.substr(path.find_last_of(".") + 1);
    return sceneLoaders[extension](device, path, config);
}
