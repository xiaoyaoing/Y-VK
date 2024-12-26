// Created by pc on 2023/11/29.
//

#ifndef VULKANDEMO_RENDERPRIMITIVE_H
#define VULKANDEMO_RENDERPRIMITIVE_H
#include "Core/BoundingBox.h"
#include "Core/Buffer.h"
#include "Core/Transform.h"

class Material;

enum class PRIMITIVE_TYPE : uint8_t {
    E_TRIANGLE_LIST,
    E_POINT_LIST,
};

VkPrimitiveTopology GetVkPrimitiveTopology(PRIMITIVE_TYPE type);

struct VertexAttribute {
    VkFormat      format = VK_FORMAT_UNDEFINED;
    std::uint32_t stride = 0;
    std::uint32_t offset = 0;
};

#define POSITION_ATTRIBUTE_NAME "position"
#define INDEX_ATTRIBUTE_NAME    "indices"
#define NORMAL_ATTRIBUTE_NAME   "normal"
#define TANGENT_ATTRIBUTE_NAME  "tangent"
#define TEXCOORD_ATTRIBUTE_NAME "texcoord_0"

struct PerPrimitiveUniform {
    glm::mat4 model;
    glm::mat4 modelIT;
    uint32_t  materialIndex;
    uint32_t  padding1;
    uint32_t  padding2;
    uint32_t  padding3;
};

class Primitive {
protected:
    //ShaderVarint varint{};

    VkIndexType indexType{VK_INDEX_TYPE_UINT16};

    BBox transformedDimensions;
    BBox originalDimensions;

    std::unordered_map<std::string, VertexAttribute>         vertexAttributes;
    std::unordered_map<std::string, std::unique_ptr<Buffer>> vertexBuffers;
    std::unique_ptr<Buffer>                                  indexBuffer;
    std::unique_ptr<Buffer>                                  uniformBuffer;

public:
    PRIMITIVE_TYPE primitiveType{PRIMITIVE_TYPE::E_TRIANGLE_LIST};
    uint32_t       firstIndex{0};
    uint32_t       indexCount{};
    uint32_t       firstVertex{0};
    uint32_t       vertexCount{};
    uint32_t       materialIndex{0};
    Transform      transform{};
    uint32_t       lightIndex{-1u};

    glm::mat4 getTransformMatrix() const {
        return transform.getLocalToWorldMatrix();
    }
    bool getVertexAttribute(const std::string& name, VertexAttribute* attribute = nullptr) const;
    void setVertxAttribute(const std::string& name, const VertexAttribute& attribute);
    void setVertexBuffer(const std::string& name, std::unique_ptr<Buffer>& buffer);
    // void        setVertexBuffer(const std::string& name, std::unique_ptr<Buffer> buffer);
    void setUniformBuffer(std::unique_ptr<Buffer>& buffer);
    void setIndexBuffer(std::unique_ptr<Buffer>& buffer);
    // void        setIndexBuffer(std::unique_ptr<Buffer> buffer);
    bool        valid() const;
    Buffer&     getVertexBuffer(const std::string& name) const;
    VkIndexType getIndexType() const;
    void        setIndexType(VkIndexType indexType);
    bool        hasVertexBuffer(const std::string& name) const;
    Buffer&     getIndexBuffer() const;
    bool        hasIndexBuffer() const;
    Buffer&     getUniformBuffer() const;

    // void        setDimensions(glm::vec3 min, glm::vec3 max);
    // void        setDimensions(const BBox& box);
    void        setOriginalDimensions(const BBox& box);
    void        setTransform(const Transform& transform);
    const BBox& getDimensions() const;

    Primitive(uint32_t firstVertex, uint32_t firstIndex, uint32_t vertexCount, uint32_t indexCount, uint32_t materialIndex = 0) : firstIndex(firstIndex),
                                                                                                                                  indexCount(indexCount), firstVertex(firstVertex), vertexCount(vertexCount),
                                                                                                                                  materialIndex(materialIndex), transformedDimensions({}) {
    }

    Primitive(uint32_t firstVertex, uint32_t vertexCount, uint32_t materialIndex) : firstVertex(firstVertex), vertexCount(vertexCount), materialIndex(materialIndex), transformedDimensions({}) {
    }
    PerPrimitiveUniform GetPerPrimitiveUniform() const {
        return {transform.getLocalToWorldMatrix(), glm::transpose(glm::inverse(transform.getLocalToWorldMatrix())), materialIndex, 0, 0, 0};
    } 
};

#endif//VULKANDEMO_RENDERPRIMITIVE_H