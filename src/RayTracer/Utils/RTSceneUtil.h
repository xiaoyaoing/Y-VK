#pragma once
#include "Core/RayTracing/Accel.h"
#include "Scene/Scene.h"
#include "shaders/Raytracing/commons.h"

struct BlasInput
{
    std::vector<VkAccelerationStructureGeometryKHR> geometry;
    std::vector<VkAccelerationStructureBuildRangeInfoKHR> range;
};


using TlasInput  = VkAccelerationStructureInstanceKHR;





struct RTSceneEntry
{
    std::unique_ptr<Buffer> vertexBuffer{nullptr};
    std::unique_ptr<Buffer> normalBuffer{nullptr};
    std::unique_ptr<Buffer> uvBuffer{nullptr};
    std::unique_ptr<Buffer> indexBuffer{nullptr};
    std::unique_ptr<Buffer> materialsBuffer{nullptr};
    std::unique_ptr<Buffer> sceneDescBuffer{nullptr};
    std::unique_ptr<Buffer> primitiveMeshBuffer{nullptr};
    std::unique_ptr<Buffer> rtLightBuffer{nullptr};

    std::vector<Buffer> transformBuffers{};

    std::vector<Accel> blases;
    Accel tlas;
    
    std::vector<RTLight> lights;
    std::vector<RTPrimitive> primitives;
    std::vector<RTMaterial> materials;

    std::vector<Texture *> textures{};
};

class RTSceneUtil
{
public:
    static std::unique_ptr<RTSceneEntry> convertScene(Device& device, Scene & scene);
};
