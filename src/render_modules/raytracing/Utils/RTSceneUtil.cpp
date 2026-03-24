#include "RTSceneUtil.h"

#include "../scene/RTSceneBuilder.h"

std::unique_ptr<RTSceneEntry> RTSceneUtil::convertScene(Device& device, Scene& scene) {
    return RTSceneBuilder::build(device, scene);
}
