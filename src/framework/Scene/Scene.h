#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include "Compoments/RenderPrimitive.h"
#include "Core/Texture.h"
#include "Core/Buffer.h"

#include "Scene/Compoments/SgLight.h"

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
};

class Scene {
public:
    using PrimitiveCallBack = std::function<void(const Primitive& primitive)>;

    void IteratePrimitives(PrimitiveCallBack primitiveCallBack) const;

    Scene(std::vector<std::unique_ptr<Primitive>>&& primitives, std::vector<std::unique_ptr<Texture>>&& textures, std::vector<Material>&& materials, std::vector<SgLight>&& lights, std::vector<std::shared_ptr<Camera>>&& cameras);

    void addLight(const SgLight& light);

    const std::vector<SgLight>& getLights() const;

    const std::vector<std::unique_ptr<Primitive>>& getPrimitives() const;
    const std::vector<std::unique_ptr<Texture>>&   getTextures() const;
    const std::vector<Material>&                   getMaterials() const;

    std::vector<std::shared_ptr<Camera>>& getCameras();

private:
    std::vector<Material> materials;

    std::vector<SgLight> lights;

    std::vector<std::unique_ptr<Primitive>> primitives;
    std::vector<std::unique_ptr<Texture>>   textures;

    std::vector<std::shared_ptr<Camera>> cameras;
};