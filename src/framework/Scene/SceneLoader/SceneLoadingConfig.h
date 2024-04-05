

#include <string>
#include <unordered_set>
#include <vulkan/vulkan_core.h>
struct SceneLoadingConfig {
    std::unordered_set<std::string> requiredVertexAttribute{};
    VkIndexType                     indexType{VK_INDEX_TYPE_NONE_KHR};
    bool                            bufferAddressAble{false};
    bool                            bufferForAccel{false};
    bool                            bufferForTransferSrc{false};
    bool                            bufferForTransferDst{false};
};