//
// Created by pc on 2023/11/29.
//

#ifndef VULKANDEMO_RENDERPRIMITIVE_H
#define VULKANDEMO_RENDERPRIMITIVE_H
#include "Core/Buffer.h"

class Material;

enum  class PRIMITIVE_TYPE : uint8_t
{
    E_TRIANGLE_LIST,
};

struct VertexAttribute {
    VkFormat format = VK_FORMAT_UNDEFINED;

    std::uint32_t stride = 0;

    std::uint32_t offset = 0;
};

#define POSITION_ATTRIBUTE_NAME "position"
#define INDEX_ATTRIBUTE_NAME "indices"
#define NORMAL_ATTRIBUTE_NAME "normal"
#define TANGENT_ATTRIBUTE_NAME "tangent"
#define TEXCOORD_ATTRIBUTE_NAME "texcoord"

struct Primitive {
    uint32_t firstIndex{};
    uint32_t indexCount{};
    uint32_t firstVertex{};
    uint32_t vertexCount{};
    Material &material;
    glm::mat4 matrix{};
    uint32_t materialIndex{0};

    PRIMITIVE_TYPE primitiveType{PRIMITIVE_TYPE::E_TRIANGLE_LIST};

    VkIndexType indexType{VK_INDEX_TYPE_UINT16};

    struct Dimensions {
        glm::vec3 min = glm::vec3(FLT_MAX);
        glm::vec3 max = glm::vec3(-FLT_MAX);
        glm::vec3 size;
        glm::vec3 center;
        float radius;
    } dimensions;

    std::unordered_map<std::string, VertexAttribute> vertexAttributes;

    std::unordered_map<std::string, std::unique_ptr<Buffer>> vertexBuffers;

    std::unique_ptr<Buffer> indexBuffer;

    bool getVertexAttribute(const std::string &name, VertexAttribute &attribute) const;

    void setVertxAttribute(const std::string &name, VertexAttribute &attribute);

    void setVertexBuffer(const std::string &name, std::unique_ptr<Buffer> &buffer);

    Buffer &getVertexBuffer(const std::string &name) const;

    void setDimensions(glm::vec3 min, glm::vec3 max);

    Primitive(uint32_t firstIndex, uint32_t indexCount, Material &material) : firstIndex(firstIndex),
                                                                              indexCount(indexCount),
                                                                              material(material), dimensions({}) {
    }
};

#endif //VULKANDEMO_RENDERPRIMITIVE_H
