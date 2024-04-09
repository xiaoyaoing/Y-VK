#pragma once
#include "Scene/Scene.h"

#include <string>
#include <unordered_set>
#include <volk.h>
struct SceneLoadingConfig {
    std::unordered_set<std::string> requiredVertexAttribute{"position", "normal", "texcoord_0", "indices"};
    VkIndexType                     indexType{VK_INDEX_TYPE_NONE_KHR};
    bool                            bufferAddressAble{false};
    bool                            bufferForAccel{false};
    bool                            bufferForTransferSrc{false};
    bool                            bufferForTransferDst{false};
    BufferRate                      bufferRate{BufferRate::PER_SCENE};
    glm::vec3 sceneTranslation{0.0f};
    glm::quat sceneRotation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 sceneScale{1.0f};
};