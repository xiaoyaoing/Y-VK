#include "ObjLoader.hpp"
// #include <>
// template<glm::length_t size>
// glm::vec<size, float, glm::defaultp> loadVector(const char* s) {
//     std::istringstream                   ss(s);
//     glm::vec<size, float, glm::defaultp> result;
//     for (unsigned i = 0; i < size && !ss.eof() && !ss.fail(); ++i)
//         ss >> result[i];
//     return result;
// }

// void ObjLoader::loadFace(const char* line) {
//     uint32_t first = 0, current = 0;
//     int    vertexCount = 0;
//
//     std::istringstream ss(line);
//     while (!ss.fail() && !ss.eof()) {
//         int32 indices[] = {0, 0, 0};
//         for (int i = 0; i < 3; ++i) {
//             if (ss.peek() != '/')
//                 ss >> indices[i];
//             if (ss.peek() == '/')
//                 ss.get();
//             else
//                 break;
//         }
//         if (indices[0] == 0)
//             break;
//
//         uint32_t vert = fetchVertex(indices[0], indices[2], indices[1]);
//
//         if (++vertexCount >= 3)
//             _tris.emplace_back(first, current, vert, _currentMaterial);
//         else
//             first = current;
//         current = vert;
//     }
// }
//
// ObjLoader::ObjLoader(std::ifstream& stream) {
//     loadFile(stream);
// }
//
// void ObjLoader::skipWhitespace(const char*& s) {
//     while (std::isspace(*s))
//         s++;
// }
//
// bool ObjLoader::hasPrefix(const char* s, const char* pre) {
//     do {
//         if (tolower(*pre) != tolower(*s))
//             return false;
//     } while (*++s && *++pre);
//     return *pre == '\0' && (isspace(*s) || *s == '\0');
// }
//
//
// void ObjLoader::loadFile(std::ifstream& stream) {
//     std::string meshLine;
//     while (!stream.eof() && !stream.fail()) {
//         std::getline(stream, meshLine);
//         loadLine(meshLine.c_str());
//     }
// }
//
// void ObjLoader::loadLine(const char* line) {
//     skipWhitespace(line);
//     if (hasPrefix(line, "v"))
//         _pos.push_back(loadVector<3>(line + 2));
//     else if (hasPrefix(line, "vn"))
//         _normal.push_back(loadVector<3>(line + 3));
//     else if (hasPrefix(line, "vt"))
//         _uv.push_back(loadVector<2>(line + 3));
//     else if (hasPrefix(line, "f"))
//         loadFace(line + 2);
// }
//
// uint32_t ObjLoader::fetchVertex(int32 pos, int32 normal, int32 uv) {
//     if (pos < 0)
//         pos += _pos.size() + 1;
//     if (normal < 0)
//         normal += _normal.size() + 1;
//     if (uv < 0)
//         uv += _uv.size() + 1;
//
//     auto iter = _indices.find(iglm::vec3(pos, normal, uv));
//     if (iter != _indices.end()) {
//         return iter->second;
//     } else {
//         glm::vec3 p(0.0f), n(0.0f, 1.0f, 0.0f);
//         glm::vec2 u(0.0f);
//
//         if (pos)
//             p = _pos[pos - 1];
//         if (normal)
//             n = _normal[normal - 1];
//         if (uv)
//             u = _uv[uv - 1];
//
//         // _bounds.grow(p);
//         uint32_t index = _verts.size();
//         _verts.emplace_back(p, n, u);
//         _indices.insert(std::make_pair(iglm::vec3(pos, normal, uv), index));
//         return index;
//     }
// }

struct TriangleI {
    union {
        struct {
            uint32 v0, v1, v2;
        };
        uint32 vs[3];
    };
    uint32_t material;
    TriangleI(uint32 _v0, uint32 _v1, uint32 _v2, uint32_t _material) : v0(_v0), v1(_v1), v2(_v2), material(_material) {}
    TriangleI() = default;
};

struct Vertex {
    glm::vec3 _pos, _normal;
    glm::vec2 _uv;

    Vertex(const glm::vec3& pos, const glm::vec3& normal, const glm::vec2& uv) : _pos(pos), _normal(normal), _uv(uv) {}
    Vertex() {}
    const glm::vec3& normal() const {
        return _normal;
    }

    const glm::vec3& pos() const {
        return _pos;
    }

    const glm::vec2& uv() const {
        return _uv;
    }

    glm::vec3& normal() {
        return _normal;
    }

    glm::vec3& pos() {
        return _pos;
    }

    glm::vec2& uv() {
        return _uv;
    }
};

std::unique_ptr<PrimitiveData> loadWo3(std::ifstream &stream) {
    std::vector<Vertex> vertexs;
    std::vector<TriangleI> tris;
    uint64_t numVerts, numTris;
    FileUtils::streamRead(stream, numVerts);
    vertexs.resize(size_t(numVerts));
    FileUtils::streamRead(stream, vertexs);
    FileUtils::streamRead(stream, numTris);
    tris.resize(size_t(numTris));
    FileUtils::streamRead(stream, tris);

    auto primitiveData = std::make_unique<PrimitiveData>();
    primitiveData->buffers[POSITION_ATTRIBUTE_NAME] = {};
    primitiveData->buffers[NORMAL_ATTRIBUTE_NAME] = {};
    primitiveData->buffers[TEXCOORD_ATTRIBUTE_NAME] = {};
    primitiveData->indexs = {};

    primitiveData->buffers[POSITION_ATTRIBUTE_NAME].reserve(vertexs.size() * sizeof(glm::vec3));
    primitiveData->buffers[NORMAL_ATTRIBUTE_NAME].reserve(vertexs.size() * sizeof(glm::vec3));
    primitiveData->buffers[TEXCOORD_ATTRIBUTE_NAME].reserve(vertexs.size() * sizeof(glm::vec2));
    primitiveData->indexs.reserve(tris.size() * 3 * sizeof(uint32_t));

    for (uint32_t i = 0; i < vertexs.size(); i++) {
        primitiveData->buffers[POSITION_ATTRIBUTE_NAME].insert(primitiveData->buffers[POSITION_ATTRIBUTE_NAME].end(), (uint8_t*)&vertexs[i]._pos, (uint8_t*)&vertexs[i]._pos + sizeof(glm::vec3));
        primitiveData->buffers[NORMAL_ATTRIBUTE_NAME].insert(primitiveData->buffers[NORMAL_ATTRIBUTE_NAME].end(), (uint8_t*)&vertexs[i]._normal, (uint8_t*)&vertexs[i]._normal + sizeof(glm::vec3));
        primitiveData->buffers[TEXCOORD_ATTRIBUTE_NAME].insert(primitiveData->buffers[TEXCOORD_ATTRIBUTE_NAME].end(), (uint8_t*)&vertexs[i]._uv, (uint8_t*)&vertexs[i]._uv + sizeof(glm::vec2));
    }

    for (uint32_t i = 0; i < tris.size(); i++) {
        primitiveData->indexs.insert(primitiveData->indexs.end(), (uint8_t*)&tris[i], (uint8_t*)&tris[i] + 3 * sizeof(uint32_t));
    } 
    auto ivec3 = reinterpret_cast<glm::ivec3*>(primitiveData->indexs.data());
    primitiveData->vertexAttributes[POSITION_ATTRIBUTE_NAME] = VertexAttribute{VK_FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec3)};
    primitiveData->vertexAttributes[NORMAL_ATTRIBUTE_NAME] = VertexAttribute{VK_FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec3)};
    primitiveData->vertexAttributes[TEXCOORD_ATTRIBUTE_NAME] = VertexAttribute{VK_FORMAT_R32G32_SFLOAT, sizeof(glm::vec2)};
    return primitiveData;
}

std::unique_ptr<PrimitiveData> PrimitiveLoader::loadPrimitive(const std::filesystem::path& path) {
    std::ifstream stream(path.c_str(), std::ios::binary);
    auto          s = path.string();
    if (!stream.is_open()) {
        LOGE("Failed to open file: {}", path.string());
        return {};
    }
    std::string ext = FileUtils::getFileExt(path.string());
    if (ext == "wo3") {
        return loadWo3(stream);
    }
    LOGE("Unsupported file format: %s", ext.c_str());
    return {};
}
std::unique_ptr<PrimitiveData> PrimitiveLoader::loadPrimitiveFromType(const std::string& type) {
    return nullptr;
}