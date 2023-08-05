//
// Created by 打工人 on 2023/3/18.
//
#include "ext/tinyobjloader/tiny_obj_loader.h"
#include "Mesh.h"
#include "CommandBuffer.h"

Mesh::Mesh(const char *path) {
    ASSERT(LoadData(path), "Failed to Load Mesh Data");
}

bool Mesh::LoadData(const char *path) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path)) {
        throw std::runtime_error(warn + err);
    }
    for (const auto &shape: shapes) {
        for (const auto &index: shape.mesh.indices) {
            Vertex vertex{};
            vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]};
            vertex.uv = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1 - attrib.texcoords[2 * index.texcoord_index + 1]};
            vertex.color = {1.0f, 1.0f, 1.0f};
            _vertexes.push_back(vertex);
            _indices.push_back(_indices.size());
        }
    }
    return true;
}

void Mesh::createBuffer(VmaAllocator allocator) {
    _vertexBuffer = std::make_shared<Buffer>(allocator, DATA_SIZE(_vertexes), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                             VMA_MEMORY_USAGE_CPU_TO_GPU);
    _vertexBuffer->uploadData(_vertexes.data(), DATA_SIZE(_vertexes));
    _indicesBuffer = std::make_shared<Buffer>(allocator, DATA_SIZE(_indices), VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                              VMA_MEMORY_USAGE_CPU_TO_GPU);
    _indicesBuffer->uploadData(_indices.data(), DATA_SIZE(_indices));
}

void Mesh::bindOnly(CommandBuffer &commandBuffer) {
    const VkBuffer vertexBuffer[] = {_vertexBuffer->getHandle()};
    const VkDeviceSize vertexOffset[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffer, vertexOffset);
    vkCmdBindIndexBuffer(commandBuffer, _indicesBuffer->getHandle(), 0, VK_INDEX_TYPE_UINT32);

}

void Mesh::drawOnly(CommandBuffer &commandBuffer) {
    vkCmdDrawIndexed(commandBuffer, _indices.size(), 1, 0, 0, 0);
}

void Mesh::bindAndDraw(CommandBuffer &commandBuffer) {
    bindOnly(commandBuffer);
    drawOnly(commandBuffer);
}
