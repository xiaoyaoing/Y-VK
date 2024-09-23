#pragma once

#include <vec3.hpp>
#include <vec4.hpp>

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

using LoadCallback = std::function<void()>;


class SceneLoadCompleteInfo {
protected:
    bool sceneGeometryLoaded{false};
    bool sceneTexturesLoaded{false};
    LoadCallback * callback{nullptr};
public:
    bool GetSceneLoaded() const {
        return sceneGeometryLoaded && sceneTexturesLoaded;
    }
    void SetTextureLoaded() {
        sceneTexturesLoaded = true;
        if (GetSceneLoaded() && callback) {
            callback->operator()();
            callback = nullptr;
        }
    }
    void SetGeometryLoaded() {
        sceneGeometryLoaded = true;
        if (GetSceneLoaded() && callback) {
            callback->operator()();
            callback = nullptr;
        }
    }
    void SetCallback(LoadCallback * callback) {
        this->callback = callback;
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
    void addDirectionalLight(vec3 direction, vec3 color, float intensity, vec3 position);

    const std::vector<SgLight>& getLights() const;
    std::vector<SgLight>& getLights();

    BBox getSceneBBox() const;
    void setSceneBBox(const BBox& bbox);
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
    void setTextures(std::vector<std::unique_ptr<Texture>>&& textures);
    bool getMergeDrawCall() const;
    void setMergeDrawCall(bool mergeDrawCall);

    void addPrimitive(std::unique_ptr<Primitive> primitive);
    void addPrimitives(std::vector<std::unique_ptr<Primitive>>&& primitives);
    void setName(const std::string& name);
    const std::string& getName() const;

    SceneLoadCompleteInfo& getLoadCompleteInfo() const;

protected:
    friend GltfLoading;
    friend Jsonloader;
    friend RuntimeSceneManager;
    bool mergeDrawCall = false;
    std::string mName;
    std::vector<GltfMaterial> materials;
    std::vector<RTMaterial>   rtMaterials;
    BBox sceneBBox;

    std::vector<SgLight> lights;

    std::vector<std::unique_ptr<Primitive>> primitives;
    std::vector<std::unique_ptr<Texture>>   textures;
    std::vector<std::shared_ptr<Camera>>    cameras;

    std::unordered_map<std::string, VertexAttribute>         vertexAttributes;
    std::unordered_map<std::string, std::unique_ptr<Buffer>> sceneVertexBuffer;
    std::unique_ptr<Buffer>                                  sceneIndexBuffer{nullptr};
   std::unique_ptr<Buffer>                                  sceneUniformBuffer{nullptr};
    // std::vector<std::unique_ptr<Buffer>>                                  sceneUniformBuffer{};
    VkIndexType                                              indexType{VK_INDEX_TYPE_UINT16};

    std::unique_ptr<Buffer> primitiveIdBuffer{};
    bool                    usePrimitiveId{true};
    std::string             sceneFilePath;
    BufferRate              bufferRate{BufferRate::PER_SCENE};

    std::unique_ptr<SceneLoadCompleteInfo> loadCompleteInfo;
};

std::unique_ptr<Scene> loadDefaultTriangleScene(Device& device);
GltfMaterial           InitGltfMaterial();
