//
// Created by pc on 2023/8/27.
//

#pragma once

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ext/tinygltf/tiny_gltf.h"

#include <filesystem>
#include <unordered_set>

#include "Core/Buffer.h"
#include "Core/Texture.h"
#include "Scene/Compoments/Camera.h"
#include "Scene/Scene.h"
//#include "Mesh.h"

struct SceneLoadingConfig
{
    std::unordered_set<std::string> requiredVertexAttribute{};
    VkIndexType indexType{VK_INDEX_TYPE_NONE_KHR};
    bool bufferAddressAble{false};
    bool bufferForAccel{false};
    bool bufferForTransferSrc{false};
    bool bufferForTransferDst{false};
};

class GltfLoading
{
public:

    
    static std::unique_ptr<Scene> LoadSceneFromGLTFFile(Device& device, const std::string& path,const SceneLoadingConfig & config = {});

    enum FileLoadingFlags
    {
        None = 0x00000000,
        PreTransformVertices = 0x00000001,
        PreMultiplyVertexColors = 0x00000002,
        FlipY = 0x00000004,
        DontLoadImages = 0x00000008
    };

    enum RenderFlags
    {
        BindImages = 0x00000001,
        RenderOpaqueNodes = 0x00000002,
        RenderAlphaMaskedNodes = 0x00000004,
        RenderAlphaBlendedNodes = 0x00000008
    };
};
