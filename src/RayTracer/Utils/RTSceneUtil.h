#pragma once
#include "Core/RayTracing/Accel.h"
#include "Raytracing/commons.h"
#include "Scene/Scene.h"

struct BlasInput {
    std::vector<VkAccelerationStructureGeometryKHR>       geometry;
    std::vector<VkAccelerationStructureBuildRangeInfoKHR> range;
};

using TlasInput = VkAccelerationStructureInstanceKHR;

struct RTSceneEntry {
    Buffer* vertexBuffer{nullptr};
    Buffer* normalBuffer{nullptr};
    Buffer* uvBuffer{nullptr};
    Buffer* indexBuffer{nullptr};

    std::unique_ptr<Buffer> materialsBuffer{nullptr};
    std::unique_ptr<Buffer> sceneDescBuffer{nullptr};
    std::unique_ptr<Buffer> primitiveMeshBuffer{nullptr};
    std::unique_ptr<Buffer> rtLightBuffer{nullptr};

    std::vector<Buffer>                  transformBuffers{};
    std::vector<std::unique_ptr<Buffer>> primAreaDistributionBuffers{};

    std::vector<Accel> blases;
    Accel              tlas;

    std::vector<RTLight>     lights;
    std::vector<RTPrimitive> primitives;
    std::vector<RTMaterial>  materials;

    std::vector<Texture*> textures{};
};

class RTSceneUtil {
public:
    static std::unique_ptr<RTSceneEntry> convertScene(Device& device, Scene& scene);
};
