#pragma once
#include "Scene/Compoments/RenderPrimitive.h"

#include <filesystem>
#include <fstream>
#include <vector>
#include <unordered_map>

#include <glm.hpp>


using BufferData = std::vector<uint8_t>;

struct PrimitiveData { 
    std::unordered_map<std::string,BufferData> buffers;
    std::unordered_map<std::string, VertexAttribute>         vertexAttributes;
    BufferData indexs;
    BBox bbox;
};


namespace PrimitiveLoader {
    enum  PrimitiveType {
        QUAD,
        DISK,
        SPHERE,
        CUBE,
    };
    std::unique_ptr<PrimitiveData> loadPrimitive(const std::filesystem::path& path);
    std::unique_ptr<PrimitiveData> loadPrimitiveFromType(const std::string & type);
    std::unique_ptr<PrimitiveData> loadPrimitive(PrimitiveType type);
}