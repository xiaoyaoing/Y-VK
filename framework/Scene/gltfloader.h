//
// Created by pc on 2023/8/27.
//

#pragma once

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ext/tinygltf/tiny_gltf.h"

#include <filesystem>

#include "Buffer.h"
#include "API_VK.h"
#include "Camera.h"
#include "Scene.h"
//#include "Mesh.h"


class GltfLoading
{
public:
    static std::unique_ptr<Scene> LoadSceneFromGLTFFile(Device& device, const std::string& path,
                                                        uint32_t fileLoadingFlags = 0, float scale = 1.0f);

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
