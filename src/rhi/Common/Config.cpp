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

public:
    Json json;

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
    void cameraFromConfig(Camera& camera,const std::string & name) {
        if (!json.contains(name) ||  !json.contains("camera"))
            return;
        const auto& cameraJson = json[name]["camera"];
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
        if (cameraJson.contains("speed")) {
            float speed = cameraJson["speed"];
            camera.setMoveSpeed(speed);
        }
    }
    void cameraToConfig(const Camera& camera,const std::string & name) {
        Json      cameraJson;
        glm::vec4 perspective     = camera.getPerspectiveParams();
        cameraJson["perspective"] = {perspective.x, perspective.y, perspective.z, perspective.w};
        cameraJson["transform"]   = to_json(*camera.getTransform());
        cameraJson["flipy"]       = camera.flipY;
        cameraJson["speed"]       = camera.getMoveSpeed();
        json[name] = Json();
        json[name]["camera"]            = cameraJson;
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
void Config::CameraFromConfig(Camera& camera,const std::string & name) {
    impl->cameraFromConfig(camera,name);
}
void Config::CameraToConfig(const Camera& camera,const std::string & name) {
    impl->cameraToConfig(camera,name);
}
const std::string Config::GetScenePath() {
    return impl->json["scene"];
}
Config::Config() {
    impl = new Impl();
}