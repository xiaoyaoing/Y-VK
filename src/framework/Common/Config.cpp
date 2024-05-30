#include "Config.h"

#include "FIleUtils.h"

#include <Common/JsonUtil.h>
Config* Config::instance = nullptr;

Json to_json(const Transform& transform) {
    Json json;
    json["position"] = {transform.getPosition().x, transform.getPosition().y, transform.getPosition().z};
    json["rotation"] = {transform.getRotation().w, transform.getRotation().x, transform.getRotation().y, transform.getRotation().z};
    json["scale"]    = {transform.getLocalScale().x, transform.getLocalScale().y, transform.getLocalScale().z};
    return json;
}

class Config::Impl {
protected:
    Json json;

public:
    Impl() {
        std::string path = FileUtils::getResourcePath("config.json");
        if (FileUtils::fileExists(path)) {
            json = JsonUtil::fromFile(path);
        }
    }
    void save() {
        std::string path = FileUtils::getResourcePath("config.json");
        JsonUtil::toFile(path, json);
    }
    void cameraFromConfig(Camera& camera) {
        if (!json.contains("camera"))
            return;
        const auto& cameraJson = json["camera"];
        if (cameraJson.contains("perspective")) {
            auto perspective              = cameraJson["perspective"];
            auto [fov, aspect, near, far] = perspective.get<std::tuple<float, float, float, float>>();
            camera.setPerspective(fov, aspect, near, far);
        }
        if (cameraJson.contains("transform")) {
            auto  transform       = cameraJson["transform"];
            auto& cameraTransform = *camera.getTransform();
            auto& position        = transform["position"];
            auto& rotation        = transform["rotation"];
            auto& scale           = transform["scale"];
            cameraTransform.setPosition({position[0], position[1], position[2]});
            cameraTransform.setRotation({rotation[0], rotation[1], rotation[2], rotation[3]});
            cameraTransform.setLocalScale({scale[0], scale[1], scale[2]});
            camera.updateViewMatrix();
        }
        if (cameraJson.contains("flipy")) {
            bool filpy = cameraJson["flipy"];
            camera.setFlipY(filpy);
        }
    }
    void cameraToConfig(const Camera& camera) {
        Json      cameraJson;
        glm::vec4 perspective     = camera.getPerspectiveParams();
        cameraJson["perspective"] = {perspective.x, perspective.y, perspective.z, perspective.w};
        cameraJson["transform"]   = to_json(*camera.getTransform());
        cameraJson["flipy"]       = camera.flipY;
        json["camera"]            = cameraJson;
    }
};

void Config::SaveConfig() {
    impl->save();
}
Config Config::GetInstance() {
    if (instance == nullptr) {
        instance = new Config();
    }
    return *instance;
}
void Config::CameraFromConfig(Camera& camera) {
    impl->cameraFromConfig(camera);
}
void Config::CameraToConfig(const Camera& camera) {
    impl->cameraToConfig(camera);
}
Config::Config() {
    impl = new Impl();
}