#pragma once
#include "Scene/Compoments/Camera.h"

class Config {
public:
    void SaveConfig();
    Config static GetInstance();
    void              CameraFromConfig(Camera& camera,const std::string & name);
    void              CameraToConfig(const Camera& camera,const std::string & name);
    const std::string GetScenePath();

protected:
    Config();
    static Config* instance;
    class Impl;
    Impl* impl{nullptr};
};
