#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <API_VK.h>
#include <Buffer.h>

class Camera;

struct Material
{
    enum AlphaMode
    {
        ALPHAMODE_OPAQUE,
        ALPHAMODE_MASK,
        ALPHAMODE_BLEND
    };

    AlphaMode alphaMode = ALPHAMODE_OPAQUE;
    float alphaCutoff = 1.0f;
    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;
    glm::vec4 baseColorFactor = glm::vec4(1.0f);

    std::unordered_map<std::string, const Texture&> textures{};
};

struct VertexAttribute
{
    VkFormat format = VK_FORMAT_UNDEFINED;

    std::uint32_t stride = 0;

    std::uint32_t offset = 0;
};

struct Primitive
{
    uint32_t firstIndex{};
    uint32_t indexCount{};
    uint32_t firstVertex{};
    uint32_t vertexCount{};
    Material& material;
    glm::mat4 matrix{};

    struct Dimensions
    {
        glm::vec3 min = glm::vec3(FLT_MAX);
        glm::vec3 max = glm::vec3(-FLT_MAX);
        glm::vec3 size;
        glm::vec3 center;
        float radius;
    } dimensions;

    std::unordered_map<std::string, VertexAttribute> vertexAttributes;

    std::unordered_map<std::string, std::unique_ptr<Buffer>> vertexBuffers;

    std::unique_ptr<Buffer> indexBuffer;

    bool getVertexAttribute(const std::string& name, VertexAttribute& attribute) const;

    void setVertxAttribute(const std::string& name, VertexAttribute& attribute);

    void setVertexBuffer(const std::string& name, std::unique_ptr<Buffer>& buffer);

    Buffer& getVertexBuffer(const std::string& name) const;

    void setDimensions(glm::vec3 min, glm::vec3 max);

    Primitive(uint32_t firstIndex, uint32_t indexCount, Material& material) : firstIndex(firstIndex),
                                                                              indexCount(indexCount),
                                                                              material(material)
    {
    }
};

enum class LIGHT_TYPE : uint8_t
{
    Directional = 0,
    Point = 1,
    Spot = 2,
    // Insert new light type here
    Max
};

struct LightProperties
{
    glm::vec3 direction{0.0f, 0.0f, -1.0f};

    glm::vec3 color{1.0f, 1.0f, 1.0f};

    float intensity{1.0f};

    float range{0.0f};

    float inner_cone_angle{0.0f};

    float outer_cone_angle{0.0f};
};

struct Light
{
    LightProperties lightProperties;
    LIGHT_TYPE type;
};


class Scene
{
public:
    using PrimitiveCallBack = std::function<void(const Primitive& primitive)>;
    void IteratePrimitives(PrimitiveCallBack primitiveCallBack) const;

    Scene(std::vector<std::unique_ptr<Primitive>>&& primitives, std::vector<Texture>&& textures,
          std::vector<Material> materials, std::vector<Light> lights);

private:
    std::vector<Material> materials;
    std::vector<Light> lights;

    std::vector<std::unique_ptr<Primitive>> primitives;
    std::vector<Texture> textures;
};
