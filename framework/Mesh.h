#pragma once

#include "VertexData.h"
#include "Buffer.h"

class CommandBuffer;

class Mesh {
public:
    Mesh(const char *path);

    bool LoadData(const char *path);

    void createBuffer(Device &device);

    void bindOnly(CommandBuffer &);

    void drawOnly(CommandBuffer &);

    void bindAndDraw(CommandBuffer &);

protected:
    std::vector<Vertex> _vertexes;
    std::vector<uint32_t> _indices;
    ptr<Buffer> _vertexBuffer, _indicesBuffer;
};
