#pragma once
#include "Scene/Compoments/RenderPrimitive.h"

#include <fstream>
#include <vector>
#include <unordered_map>


#include <fwd.hpp>
#include <vec3.hpp>
class ObjLoader {
public:
    ObjLoader(std::ifstream& stream);
    void   loadFile(std::ifstream& stream);
    void   loadLine(const char* line);
    void   loadFace(const char* line);
   // uint32_t fetchVertex(int32 pos, int32 normal, int32 uv);

public:
    std::vector<glm::vec3>                 _pos, _normal;
    std::vector<glm::vec2>                 _uv;
    std::vector<glm::uint32_t>             indexs;
    uint32_t                            _currentMaterial;
    void                              skipWhitespace(const char*& s);

    bool hasPrefix(const char* s, const char* pre);
};

struct PrimitiveData { 
    std::unordered_map<std::string,std::vector<uint8_t>> buffers;
};


namespace PrimitiveLoader {
    PrimitiveData loadPrimitive(const std::string& path);
}