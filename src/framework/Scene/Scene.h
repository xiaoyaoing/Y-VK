#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "Compoments/RenderPrimitive.h"
#include "Core/Texture.h"
#include "Core/Buffer.h"

#include "Scene/Compoments/SgLight.h"
#include "../shaders/gltfMaterial.glsl"
#include "Raytracing/commons.h"

class Camera;

class GltfLoading;
class Jsonloader;
class RuntimeSceneManager;

enum class BufferRate {
    PER_PRIMITIVE,
    PER_SCENE
};

struct SceneLoadCompleteInfo {
    bool sceneGeometryLoaded{false};
    bool sceneTexturesLoaded{false};
    bool GetSceneLoaded() const {
        return sceneGeometryLoaded && sceneTexturesLoaded;
    }
};

class Scene {
public:
    Scene();
    using PrimitiveCallBack = std::function<void(const Primitive& primitive)>;

    void IteratePrimitives(PrimitiveCallBack primitiveCallBack) const;

    // Scene(std::vector<std::unique_ptr<Primitive>>&& primitives, std::vector<std::unique_ptr<Texture>>&& textures, std::vector<Material>&& materials, std::vector<SgLight>&& lights, std::vector<std::shared_ptr<Camera>>&& cameras);
    // Scene(std::vector<std::unique_ptr<Primitive>>&& primitives, std::vector<std::unique_ptr<Texture>>&& textures, std::vector<GltfMaterial>&& materials, std::vector<SgLight>&& lights, std::vector<std::shared_ptr<Camera>>&& cameras);

    void addLight(const SgLight& light);
    void addTexture(std::unique_ptr<Texture> texture);
    void addDirectionalLight(vec3 direction, vec3 color, float intensity);

    const std::vector<SgLight>& getLights() const;

    const std::vector<std::unique_ptr<Primitive>>& getPrimitives() const;
    const std::vector<std::unique_ptr<Texture>>&   getTextures() const;
    const std::vector<GltfMaterial>&               getGltfMaterials() const;
    const std::vector<RTMaterial>&                 getRTMaterials() const;
    std::vector<std::shared_ptr<Camera>>&          getCameras();
    bool                                           getVertexAttribute(const std::string& name, VertexAttribute* attribute = nullptr) const;
    Buffer&                                        getVertexBuffer(const std::string& name) const;
    VkIndexType                                    getIndexType() const;
    bool                                           hasVertexBuffer(const std::string& name) const;
    Buffer&                                        getIndexBuffer() const;
    Buffer&                                        getUniformBuffer() const;
    Buffer&                                        getPrimitiveIdBuffer() const;
    bool                                           usePrimitiveIdBuffer() const;
    void                                           updateSceneUniformBuffer();
    void                                           updateScenePrimitiveIdBuffer();
    void                                           setBufferRate(BufferRate rate);
    BufferRate                                     getBufferRate() const;

    void addPrimitive(std::unique_ptr<Primitive> primitive);
    void addPrimitives(std::vector<std::unique_ptr<Primitive>>&& primitives);

    SceneLoadCompleteInfo& getLoadCompleteInfo() const;

protected:
    friend GltfLoading;
    friend Jsonloader;
    friend RuntimeSceneManager;
    std::vector<GltfMaterial> materials;
    std::vector<RTMaterial>   rtMaterials;

    std::vector<SgLight> lights;

    std::vector<std::unique_ptr<Primitive>> primitives;
    std::vector<std::unique_ptr<Texture>>   textures;
    std::vector<std::shared_ptr<Camera>>    cameras;

    std::unordered_map<std::string, VertexAttribute>         vertexAttributes;
    std::unordered_map<std::string, std::unique_ptr<Buffer>> sceneVertexBuffer;
    std::unique_ptr<Buffer>                                  sceneIndexBuffer{nullptr};
    std::unique_ptr<Buffer>                                  sceneUniformBuffer{nullptr};
    VkIndexType                                              indexType{VK_INDEX_TYPE_UINT16};

    std::unique_ptr<Buffer> primitiveIdBuffer{};
    bool                    usePrimitiveId{true};
    std::string             sceneFilePath;
    BufferRate              bufferRate{BufferRate::PER_SCENE};

    std::unique_ptr<SceneLoadCompleteInfo> loadCompleteInfo;
};

std::unique_ptr<Scene> loadDefaultTriangleScene(Device& device);
GltfMaterial           InitGltfMaterial();
