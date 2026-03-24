#pragma once

#include "RTScene.h"

class RTSceneBuilder {
public:
    static std::unique_ptr<RTSceneEntry> build(Device& device, Scene& scene);
};
