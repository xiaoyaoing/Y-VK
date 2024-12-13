#pragma once
#include "Scene/Scene.h"

#include <string>
#include <unordered_set>



struct SceneLoadingConfig {
    std::unordered_set<std::string> requiredVertexAttribute{"position", "normal", "texcoord_0", "indices"};
    bool enableMergeDrawCalls{true};
    VkIndexType                     indexType{VK_INDEX_TYPE_NONE_KHR};
    bool                            bufferAddressAble{false};
    bool                            bufferForAccel{false};
    bool                            bufferForTransferSrc{true};
    bool                            bufferForTransferDst{true};
    bool                            bufferForStorage{false};
    BufferRate                      bufferRate{BufferRate::PER_SCENE};
    glm::vec3                       sceneTranslation{0.0f};
    glm::quat                       sceneRotation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3                       sceneScale{1.0f};
    bool loadLight{true};
    LoadCallback                    loadCallback{nullptr};
};