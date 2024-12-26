#pragma once
#include "Core/RayTracing/Accel.h"
#include "Raytracing/commons.h"
#include "Scene/Scene.h"

struct BlasInput {
    std::vector<VkAccelerationStructureGeometryKHR>       geometry;
    std::vector<VkAccelerationStructureBuildRangeInfoKHR> range;
};

using TlasInput = VkAccelerationStructureInstanceKHR;

struct EnvMapUpdateData {
    // std::shared_ptr<Buffer> infiniteSamplingBuffer{nullptr};
    std::unique_ptr<Texture> envMap{nullptr};
};

struct RTSceneEntry {
    Scene*  scene{nullptr};
    Buffer* vertexBuffer{nullptr};
    Buffer* normalBuffer{nullptr};
    Buffer* uvBuffer{nullptr};
    Buffer* indexBuffer{nullptr};

    std::shared_ptr<Buffer> materialsBuffer{nullptr};
    std::shared_ptr<Buffer> sceneDescBuffer{nullptr};
    std::shared_ptr<Buffer> primitiveMeshBuffer{nullptr};
    std::shared_ptr<Buffer> rtLightBuffer{nullptr};
    std::shared_ptr<Buffer> infiniteSamplingBuffer{nullptr};

    std::vector<Buffer> transformBuffers{};

    std::unordered_map<std::uint32_t, std::unique_ptr<Buffer>> primAreaBuffers{};

    // std::shared_ptr<Buffer> sceneDescBuffer{nullptr};
    std::shared_ptr<Buffer> sceneUboBuffer{nullptr};

    std::vector<Accel> blases;
    Accel              tlas;

    std::vector<RTLight>     lights;
    std::vector<RTPrimitive> primitives;
    std::vector<RTMaterial>  materials;

    SceneDesc sceneDesc;

    bool primAreaBuffersInitialized{false};

    virtual ~RTSceneEntry() = default;
    virtual void updateEnvMap(Device & device,EnvMapUpdateData& data) {};
};



class RTSceneUtil {
public:
    static std::unique_ptr<RTSceneEntry> convertScene(Device& device, Scene& scene);
};
