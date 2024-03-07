#pragma once

#include <vec3.hpp>
#include <vec4.hpp>

#include "Compoments/RenderPrimitive.h"
#include "Core/Texture.h"
#include "Core/Buffer.h"

#include "Scene/Compoments/SgLight.h"
#include "shaders/gltfMaterial.glsl"

class Camera;

struct Material {
    enum AlphaMode {
        ALPHAMODE_OPAQUE,
        ALPHAMODE_MASK,
        ALPHAMODE_BLEND
    };

    AlphaMode alphaMode       = ALPHAMODE_OPAQUE;
    float     alphaCutoff     = 1.0f;
    float     metallicFactor  = 1.0f;
    float     roughnessFactor = 1.0f;
    glm::vec4 baseColorFactor = glm::vec4(1.0f);
    glm::vec3 emissiveFactor  = glm::vec3(0.0f);

    std::unordered_map<std::string, const Texture&> textures{};

    static Material getDefaultMaterial();
};

class Scene {
public:
    using PrimitiveCallBack = std::function<void(const Primitive& primitive)>;

    void IteratePrimitives(PrimitiveCallBack primitiveCallBack) const;

    Scene(std::vector<std::unique_ptr<Primitive>>&& primitives, std::vector<std::unique_ptr<Texture>>&& textures, std::vector<Material>&& materials, std::vector<SgLight>&& lights, std::vector<std::shared_ptr<Camera>>&& cameras);
    Scene(std::vector<std::unique_ptr<Primitive>>&& primitives, std::vector<std::unique_ptr<Texture>>&& textures, std::vector<GltfMaterial>&& materials, std::vector<SgLight>&& lights, std::vector<std::shared_ptr<Camera>>&& cameras);

    void addLight(const SgLight& light);
    void addDirectionalLight(vec3 direction, vec3 color, float intensity);

    const std::vector<SgLight>& getLights() const;

    const std::vector<std::unique_ptr<Primitive>>& getPrimitives() const;
    const std::vector<std::unique_ptr<Texture>>&   getTextures() const;
   // const std::vector<Material>&                   getMaterials() const;
    const std::vector<GltfMaterial>&               getGltfMaterials() const;
    std::vector<std::shared_ptr<Camera>>&          getCameras();

private:
    std::vector<Material>     materials;
    std::vector<GltfMaterial> gltfMaterials;

    std::vector<SgLight> lights;

    std::vector<std::unique_ptr<Primitive>> primitives;
    std::vector<std::unique_ptr<Texture>>   textures;

    std::vector<std::shared_ptr<Camera>> cameras;
};

std::unique_ptr<Scene> loadDefaultTriangleScene(Device& device);