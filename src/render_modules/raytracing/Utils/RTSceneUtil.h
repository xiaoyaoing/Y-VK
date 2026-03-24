#pragma once

#include "../scene/RTScene.h"

class RTSceneUtil {
public:
    static std::unique_ptr<RTSceneEntry> convertScene(Device& device, Scene& scene);
};
