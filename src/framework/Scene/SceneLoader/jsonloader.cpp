#include "jsonloader.h"

#include "ObjLoader.hpp"
#include "shaders/Raytracing/commons.h"
#include <Scene/Compoments/Camera.h>

#include <nlohmann/json.hpp>
#include <tiny_obj_loader.h>

using Json = nlohmann::json;

struct JsonLoader {
    JsonLoader(Device& device) : device(device) {}
    void LoadSceneFromGLTFFile(Device& device, const std::string& path, const SceneLoadingConfig& config);
    void loadCamera();
    void loadMaterials();
    void loadPrimitives();
    Json sceneJson;
    Device & device;
    
    std::unique_ptr<Scene>                  scene;
    std::vector<std::unique_ptr<Primitive>> primitives;
    std::vector<std::unique_ptr<Texture>>   textures;
    std::vector<GltfMaterial>               materials;
    std::vector<SgLight>                    lights;
    std::vector<std::shared_ptr<Camera>>    cameras;
    std::vector<RTMaterial> rtMaterials;
};

void from_json(const Json& j, glm::vec3& v) {
    v.x = j.at(0).get<float>();
    v.y = j.at(1).get<float>();
    v.z = j.at(2).get<float>();
}

template <typename T>
T GetOptional(const Json& j, const std::string& key, const T& defaultValue) {
    auto it = j.find(key);
    if (it != j.end()) {
        return it->get<T>();
    }
    return defaultValue;
}

void JsonLoader::LoadSceneFromGLTFFile(Device& device, const std::string& path, const SceneLoadingConfig& config) {
    std::ifstream file(path);
    file >> sceneJson ;

    loadCamera();
    loadMaterials();
    loadPrimitives();
}
void JsonLoader::loadCamera() {
    std::shared_ptr<Camera> camera = std::make_shared<Camera>();
    Json cameraJson = sceneJson["camera"];
    camera->setTranslation(GetOptional(cameraJson, "translation", glm::vec3(0.0f)));
    camera->setRotation(GetOptional(cameraJson, "rotation", glm::vec3(0.0f)));
    camera->setPerspective(GetOptional(cameraJson, "fov", 45.0f), GetOptional(cameraJson, "aspect", 1.0f), GetOptional(cameraJson, "zNear", 0.1f), GetOptional(cameraJson, "zFar", 100.0f));
}

std::unordered_map<std::string,uint32_t> type2RTBSDFTYPE = {
    {"diffuse", RT_BSDF_TYPE_DIFFUSE},
    {"specular", RT_BSDF_TYPE_MIRROR}
};

void JsonLoader::loadMaterials() {
    Json materialsJson = sceneJson["materials"];
    std::unordered_set<std::string> texture_paths;
    for(auto & materialJson : materialsJson) {
        RTMaterial rtMaterial;
        if(materialJson.contains("albedo") && materialJson["albedo"].is_string()) {
            texture_paths.insert(materialJson["albedo"].get<std::string>());
        }
    }
    std::vector<std::unique_ptr<Texture>> textures;
    std::unordered_map<std::string_view,int> texture_index;
    for(auto & texture_path : texture_paths) {
        textures.push_back(Texture::loadTexture(device,texture_path));
        texture_index[texture_path] = textures.size() - 1;
    }
    for(auto & materialJson : materialsJson) {
        RTMaterial rtMaterial;
        if(materialJson.contains("albedo") && materialJson["albedo"].is_string()) {
            rtMaterial.texture_id = texture_index[materialJson["albedo"].get<std::string>()];
        }
        std::string type = materialJson["type"].get<std::string>();
        
        
        
        rtMaterials.push_back(rtMaterial);
    }
}
void JsonLoader::loadPrimitives() {
    Json primitivesJson = sceneJson["primitives"];
    for(auto & primitiveJson : primitivesJson) {
        auto primitiveData = PrimitiveLoader::loadPrimitive(primitiveJson["path"].get<std::string>());
        std::unique_ptr<Primitive> primitive = std::make_unique<Primitive>();
        primitive->setVertexBuffer(POSITION_ATTRIBUTE_NAME, Buffer::createBuffer(device,primitiveJson["vertices"].get<std::vector<glm::vec3>>()));
        primitive->setVertexBuffer(NORMAL_ATTRIBUTE_NAME, Buffer::createBuffer(device,primitiveJson["normals"].get<std::vector<glm::vec3>>()));
        primitive->setVertexBuffer(UV_ATTRIBUTE_NAME, Buffer::createBuffer(device,primitiveJson["uvs"].get<std::vector<glm::vec2>>()));
        primitive->setIndexBuffer(Buffer::createBuffer(device,primitiveJson["indices"].get<std::vector<uint32_t>>()));
        primitive->setMaterialIndex(primitiveJson["material"].get<int>());
        primitives.push_back(std::move(primitive));
    }
}
std::unique_ptr<Scene> Jsonloader::LoadSceneFromGLTFFile(Device& device, const std::string& path, const SceneLoadingConfig& config) {
    JsonLoader jsonLoader(device);
    jsonLoader.LoadSceneFromGLTFFile(device, path, config);
    return std::move(jsonLoader.scene);
}