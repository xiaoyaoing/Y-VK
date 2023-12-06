#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include "Compoments/RenderPrimitive.h"
#include "Core/Texture.h"
#include "Core/Buffer.h"

#include "Scene/Compoments/Light.h"

class Camera;

struct Material {
    enum AlphaMode {
        ALPHAMODE_OPAQUE,
        ALPHAMODE_MASK,
        ALPHAMODE_BLEND
    };

    AlphaMode alphaMode = ALPHAMODE_OPAQUE;
    float alphaCutoff = 1.0f;
    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;
    glm::vec4 baseColorFactor = glm::vec4(1.0f);

    std::unordered_map<std::string, const Texture &> textures{};
};






class Scene {
public:
    using PrimitiveCallBack = std::function<void(const Primitive &primitive)>;

    void IteratePrimitives(PrimitiveCallBack primitiveCallBack) const;

    Scene(std::vector<std::unique_ptr<Primitive>> &&primitives, std::vector<std::unique_ptr<Texture>> &&textures,
          std::vector<Material> materials, std::vector<Light> lights);

    void addLight(const Light &light);

    const std::vector<Light> &getLights() const;

    // const std::vector<Primitive> &getPrimitives() const;

    

private:
    std::vector<Material> materials;

    std::vector<Light> lights;

    std::vector<std::unique_ptr<Primitive>> primitives;
    std::vector<std::unique_ptr<Texture>> textures;
};
