#include "ObjLoader.hpp"

#include "Core/math.h"
// #include <>
template<glm::length_t size>
glm::vec<size, float, glm::defaultp> loadVector(const char* s) {
    std::istringstream                   ss(s);
    glm::vec<size, float, glm::defaultp> result;
    for (unsigned i = 0; i < size && !ss.eof() && !ss.fail(); ++i)
        ss >> result[i];
    return result;
}

struct uvec3Hash {
    size_t operator()(const glm::uvec3& v) const {
        return std::hash<int>()(v.x) ^ std::hash<int>()(v.y) ^ std::hash<int>()(v.z);
    }
    size_t operator()(const glm::ivec3& v) const {
        return std::hash<int>()(v.x) ^ std::hash<int>()(v.y) ^ std::hash<int>()(v.z);
    }
    size_t operator()(const glm::vec3& v) const {
        return std::hash<float>()(v.x) ^ std::hash<float>()(v.y) ^ std::hash<float>()(v.z);
    }
};

class ObjLoader {
public:
    ObjLoader(std::ifstream& stream);
    void loadFile(std::ifstream& stream);
    void loadLine(const char* line);
    void loadFace(const char* line);

    uint32_t fetchVertex(uint32_t pos, uint32_t normal, uint32_t uv);

public:
    std::vector<glm::vec3>                              objPos, objNormal;
    std::vector<glm::vec3>                              primPos, primNormal;
    std::vector<glm::vec2>                              objUV, primUV;
    std::vector<uint32_t>                               indexs;
    std::unordered_map<glm::uvec3, uint32_t, uvec3Hash> _indices;
    void                                                skipWhitespace(const char*& s);
    bool                                                hasPrefix(const char* s, const char* pre);
};

void ObjLoader::loadFace(const char* line) {
    uint32_t first = 0, current = 0;
    int      vertexCount = 0;

    std::istringstream ss(line);
    while (!ss.fail() && !ss.eof()) {
        uint32_t indices[] = {0, 0, 0};
        for (int i = 0; i < 3; ++i) {
            if (ss.peek() != '/')
                ss >> indices[i];
            if (ss.peek() == '/')
                ss.get();
            else
                break;
        }
        if (indices[0] == 0)
            break;

        uint32_t vert = fetchVertex(indices[0], indices[2], indices[1]);

        if (++vertexCount >= 3) {
            indexs.push_back(first);
            indexs.push_back(current);
            indexs.push_back(vert);
        } else
            first = current;
        current = vert;
    }
}

ObjLoader::ObjLoader(std::ifstream& stream) {
    loadFile(stream);
}

void ObjLoader::skipWhitespace(const char*& s) {
    while (std::isspace(*s))
        s++;
}

bool ObjLoader::hasPrefix(const char* s, const char* pre) {
    do {
        if (tolower(*pre) != tolower(*s))
            return false;
    } while (*++s && *++pre);
    return *pre == '\0' && (isspace(*s) || *s == '\0');
}

void ObjLoader::loadFile(std::ifstream& stream) {
    std::string meshLine;
    while (!stream.eof() && !stream.fail()) {
        std::getline(stream, meshLine);
        loadLine(meshLine.c_str());
    }
}

void ObjLoader::loadLine(const char* line) {
    skipWhitespace(line);
    if (hasPrefix(line, "v"))
        objPos.push_back(loadVector<3>(line + 2));
    else if (hasPrefix(line, "vn"))
        objNormal.push_back(loadVector<3>(line + 3));
    else if (hasPrefix(line, "vt"))
        objUV.push_back(loadVector<2>(line + 3));
    else if (hasPrefix(line, "f"))
        loadFace(line + 2);
}

uint32_t ObjLoader::fetchVertex(uint32_t pos, uint32_t normal, uint32_t uv) {
    if (pos < 0)
        pos += objPos.size() + 1;
    if (normal < 0)
        normal += objNormal.size() + 1;
    if (uv < 0)
        uv += objUV.size() + 1;
    auto iter = _indices.find(glm::uvec3(pos, normal, uv));
    if (iter != _indices.end()) {
        return iter->second;
    } else {
        glm::vec3 p(0.0f), n(0.0f, 1.0f, 0.0f);
        glm::vec2 u(0.0f);

        if (pos)
            p = objPos[pos - 1];
        if (normal)
            n = objNormal[normal - 1];
        if (uv)
            u = objUV[uv - 1];

        // _bounds.grow(p);
        uint32_t index = primNormal.size();
        primPos.emplace_back(p);
        primNormal.emplace_back(n);
        primUV.emplace_back(u);
        _indices.insert(std::make_pair(glm::uvec3(pos, normal, uv), index));
        return index;
    }
}

struct TriangleI {
    union {
        struct {
            uint32_t v0, v1, v2;
        };
        uint32_t vs[3];
    };
    uint32_t material;
    TriangleI(uint32_t _v0, uint32_t _v1, uint32_t _v2, uint32_t _material = -1) : v0(_v0), v1(_v1), v2(_v2), material(_material) {}
    TriangleI() = default;
};

struct Vertex {
    glm::vec3 _pos, _normal;
    glm::vec2 _uv;

    Vertex(const glm::vec3& pos, const glm::vec3& normal, const glm::vec2& uv) : _pos(pos), _normal(normal), _uv(uv) {}
    Vertex(const glm::vec3& pos, const glm::vec2& uv) : _pos(pos), _normal(0.0f, 1.0f, 0.0f), _uv(uv) {}
    Vertex(const glm::vec3& pos) : _pos(pos), _normal(0.0f, 1.0f, 0.0f), _uv(0.0f) {}
    Vertex() = default;
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

using VertAndIndex = std::pair<std::vector<Vertex>, std::vector<TriangleI>>;

std::unique_ptr<PrimitiveData> vertexIndex2PrimitiveData(const std::vector<Vertex>& verts, const std::vector<TriangleI>& tris) {
    auto primitiveData                              = std::make_unique<PrimitiveData>();
    primitiveData->buffers[POSITION_ATTRIBUTE_NAME] = {};
    primitiveData->buffers[NORMAL_ATTRIBUTE_NAME]   = {};
    primitiveData->buffers[TEXCOORD_ATTRIBUTE_NAME] = {};
    primitiveData->indexs                           = {};

    primitiveData->buffers[POSITION_ATTRIBUTE_NAME].reserve(verts.size() * sizeof(glm::vec3));
    primitiveData->buffers[NORMAL_ATTRIBUTE_NAME].reserve(verts.size() * sizeof(glm::vec3));
    primitiveData->buffers[TEXCOORD_ATTRIBUTE_NAME].reserve(verts.size() * sizeof(glm::vec2));
    primitiveData->indexs.reserve(tris.size() * 3 * sizeof(uint32_t));

    for (int i = 0; i < verts.size(); i++) {
        primitiveData->buffers[POSITION_ATTRIBUTE_NAME].insert(primitiveData->buffers[POSITION_ATTRIBUTE_NAME].end(), (uint8_t*)&verts[i]._pos, (uint8_t*)&verts[i]._pos + sizeof(glm::vec3));
        primitiveData->buffers[NORMAL_ATTRIBUTE_NAME].insert(primitiveData->buffers[NORMAL_ATTRIBUTE_NAME].end(), (uint8_t*)&verts[i]._normal, (uint8_t*)&verts[i]._normal + sizeof(glm::vec3));
        primitiveData->buffers[TEXCOORD_ATTRIBUTE_NAME].insert(primitiveData->buffers[TEXCOORD_ATTRIBUTE_NAME].end(), (uint8_t*)&verts[i]._uv, (uint8_t*)&verts[i]._uv + sizeof(glm::vec2));
    }

    for (int i = 0; i < tris.size(); i++) {
        primitiveData->indexs.insert(primitiveData->indexs.end(), (uint8_t*)&tris[i], (uint8_t*)&tris[i] + 3 * sizeof(uint32_t));
    }
    auto uvec3                                               = reinterpret_cast<glm::uvec3*>(primitiveData->indexs.data());
    primitiveData->vertexAttributes[POSITION_ATTRIBUTE_NAME] = VertexAttribute{VK_FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec3)};
    primitiveData->vertexAttributes[NORMAL_ATTRIBUTE_NAME]   = VertexAttribute{VK_FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec3)};
    primitiveData->vertexAttributes[TEXCOORD_ATTRIBUTE_NAME] = VertexAttribute{VK_FORMAT_R32G32_SFLOAT, sizeof(glm::vec2)};
}

std::unique_ptr<PrimitiveData> loadObj(std::ifstream& stream) {
    ObjLoader objLoader(stream);
    auto      primitiveData = std::make_unique<PrimitiveData>();
    primitiveData->buffers[POSITION_ATTRIBUTE_NAME].resize(objLoader.primPos.size() * sizeof(glm::vec3));
    primitiveData->buffers[NORMAL_ATTRIBUTE_NAME].resize(objLoader.primNormal.size() * sizeof(glm::vec3));
    primitiveData->buffers[TEXCOORD_ATTRIBUTE_NAME].resize(objLoader.primUV.size() * sizeof(glm::vec2));
    primitiveData->indexs.resize(objLoader.indexs.size() * sizeof(uint32_t));
    std::ranges::copy(objLoader.primPos.begin(), objLoader.primPos.end(), reinterpret_cast<glm::vec3*>(primitiveData->buffers[POSITION_ATTRIBUTE_NAME].data()));
    std::ranges::copy(objLoader.primNormal.begin(), objLoader.primNormal.end(), reinterpret_cast<glm::vec3*>(primitiveData->buffers[NORMAL_ATTRIBUTE_NAME].data()));
    std::ranges::copy(objLoader.primUV.begin(), objLoader.primUV.end(), reinterpret_cast<glm::vec2*>(primitiveData->buffers[TEXCOORD_ATTRIBUTE_NAME].data()));
    std::ranges::copy(objLoader.indexs.begin(), objLoader.indexs.end(), reinterpret_cast<uint32_t*>(primitiveData->indexs.data()));
    primitiveData->vertexAttributes[POSITION_ATTRIBUTE_NAME] = VertexAttribute{VK_FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec3)};
    primitiveData->vertexAttributes[NORMAL_ATTRIBUTE_NAME]   = VertexAttribute{VK_FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec3)};
    primitiveData->vertexAttributes[TEXCOORD_ATTRIBUTE_NAME] = VertexAttribute{VK_FORMAT_R32G32_SFLOAT, sizeof(glm::vec2)};
    return primitiveData;
}

std::unique_ptr<PrimitiveData> loadWo3(std::ifstream& stream) {
    std::vector<Vertex>    vertexs;
    std::vector<TriangleI> tris;
    uint64_t               numVerts, numTris;
    FileUtils::streamRead(stream, numVerts);
    vertexs.resize(size_t(numVerts));
    FileUtils::streamRead(stream, vertexs);
    FileUtils::streamRead(stream, numTris);
    tris.resize(size_t(numTris));
    FileUtils::streamRead(stream, tris);

    return vertexIndex2PrimitiveData(vertexs, tris);
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
    } else if (ext == "obj") {
    }
    LOGE("Unsupported file format: %s", ext.c_str());
    return {};
}

VertAndIndex makeCube() {

    std::vector<Vertex>    _verts;
    std::vector<TriangleI> _tris;

    const glm::vec3 verts[6][4] = {
        {{-0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f, 0.5f}, {0.5f, -0.5f, 0.5f}, {0.5f, -0.5f, -0.5f}},
        {{-0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}},
        {{-0.5f, 0.5f, -0.5f}, {-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}},
        {{0.5f, 0.5f, 0.5f}, {0.5f, -0.5f, 0.5f}, {-0.5f, -0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}},
        {{-0.5f, 0.5f, 0.5f}, {-0.5f, -0.5f, 0.5f}, {-0.5f, -0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f}},
        {{0.5f, 0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}},
    };
    const glm::vec2 uvs[] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};

    for (int i = 0; i < 6; ++i) {
        int idx = _verts.size();
        _tris.emplace_back(idx, idx + 2, idx + 1);
        _tris.emplace_back(idx, idx + 3, idx + 2);

        for (int j = 0; j < 4; ++j)
            _verts.emplace_back(verts[i][j], uvs[j]);
    }

    return {_verts, _tris};
}

VertAndIndex makeSphere() {
    const float            radius = 1.f;
    std::vector<Vertex>    _verts;
    std::vector<TriangleI> _tris;
    int                    SubDiv = 10;
    int                    Skip   = SubDiv * 2 + 1;
    for (int f = 0, idx = _verts.size(); f < 3; ++f) {
        for (int s = -1; s <= 1; s += 2) {
            for (int u = -SubDiv; u <= SubDiv; ++u) {
                for (int v = -SubDiv; v <= SubDiv; ++v, ++idx) {
                    glm::vec3 p(0.0f);
                    p[f]           = s;
                    p[(f + 1) % 3] = u * (1.0f / SubDiv) * s;
                    p[(f + 2) % 3] = v * (1.0f / SubDiv);
                    _verts.emplace_back(glm::normalize(p) * radius);

                    if (v > -SubDiv && u > -SubDiv) {
                        _tris.emplace_back(idx - Skip - 1, idx, idx - Skip);
                        _tris.emplace_back(idx - Skip - 1, idx - 1, idx);
                    }
                }
            }
        }
    }
    return {_verts, _tris};
}

VertAndIndex makeCylinder() {
    const float            radius = 1.f;
    const float            height = 1.f;
    std::vector<Vertex>    _verts;
    std::vector<TriangleI> _tris;
    const int              SubDiv = 36;
    int                    base   = _verts.size();
    _verts.emplace_back(glm::vec3(0.0f, -height, 0.0f));
    _verts.emplace_back(glm::vec3(0.0f, height, 0.0f));
    for (int i = 0; i < SubDiv; ++i) {
        float a = i * math::TWO_PI / SubDiv;
        _verts.emplace_back(glm::vec3(std::cos(a) * radius, -height, std::sin(a) * radius));
        _verts.emplace_back(glm::vec3(std::cos(a) * radius, height, std::sin(a) * radius));
        int i1 = (i + 1) % SubDiv;
        _tris.emplace_back(base + 0, base + 2 + i * 2, base + 2 + i1 * 2);
        _tris.emplace_back(base + 1, base + 3 + i * 2, base + 3 + i1 * 2);
        _tris.emplace_back(base + 2 + i * 2, base + 3 + i * 2, base + 2 + i1 * 2);
        _tris.emplace_back(base + 2 + i1 * 2, base + 3 + i * 2, base + 3 + i1 * 2);
    }
    return {_verts, _tris};
}

VertAndIndex makeCone() {
    const float            radius = 1.f;
    const float            height = 1.f;
    std::vector<Vertex>    _verts;
    std::vector<TriangleI> _tris;
    const int              SubDiv = 36;
    int                    base   = _verts.size();
    _verts.emplace_back(glm::vec3(0.0f));
    for (int i = 0; i < SubDiv; ++i) {
        float a = i * math::TWO_PI / SubDiv;
        _verts.emplace_back(glm::vec3(std::cos(a) * radius, height, std::sin(a) * radius));
        _tris.emplace_back(base, base + i + 1, base + ((i + 1) % SubDiv) + 1);
    }
    return {_verts, _tris};
}

//From Tungsten Render
void recomputeNormals(std::vector<Vertex>& verts, std::vector<TriangleI>& tris) {
    static const float SplitLimit = std::cos(math::PI * 0.15f);
    //static CONSTEXPR float SplitLimit = -1.0f;

    std::vector<glm::vec3>                                geometricN(verts.size(), glm::vec3(0.0f));
    std::unordered_multimap<glm::vec3, uint32, uvec3Hash> posToVert;

    for (uint32 i = 0; i < verts.size(); ++i) {
        verts[i].normal() = glm::vec3(0.0f);
        posToVert.insert(std::make_pair(verts[i].pos(), i));
    }

    for (TriangleI& t : tris) {
        const glm::vec3& p0     = verts[t.v0].pos();
        const glm::vec3& p1     = verts[t.v1].pos();
        const glm::vec3& p2     = verts[t.v2].pos();
        glm::vec3        normal = cross(p1 - p0, p2 - p0);
        if (normal == glm::vec3(0.0f))
            normal = glm::vec3(0.0f, 1.0f, 0.0f);
        else
            normal = glm::normalize(normal);

        for (int i = 0; i < 3; ++i) {
            glm::vec3& n = geometricN[t.vs[i]];
            if (n == glm::vec3(0.0f)) {
                n = normal;
            } else if (dot(n, normal) < SplitLimit) {
                verts.push_back(verts[t.vs[i]]);
                geometricN.push_back(normal);
                t.vs[i] = verts.size() - 1;
            }
        }
    }

    for (TriangleI& t : tris) {
        const glm::vec3& p0     = verts[t.v0].pos();
        const glm::vec3& p1     = verts[t.v1].pos();
        const glm::vec3& p2     = verts[t.v2].pos();
        glm::vec3        normal = cross(p1 - p0, p2 - p0);
        glm::vec3        nN     = glm::normalize(normal);

        for (int i = 0; i < 3; ++i) {
            auto iters = posToVert.equal_range(verts[t.vs[i]].pos());

            for (auto t = iters.first; t != iters.second; ++t)
                if (glm::dot(geometricN[t->second], nN) >= SplitLimit)
                    verts[t->second].normal() += normal;
        }
    }

    for (uint32 i = 0; i < verts.size(); ++i) {
        if (verts[i].normal() == glm::vec3(0.0f))
            verts[i].normal() = geometricN[i];
        else
            verts[i].normal() = glm::normalize(verts[i].normal());
    }
}

static std::unordered_map<std::string, std::function<VertAndIndex()>> primitiveMap = {
    {"cube", makeCube},
    {"sphere", makeSphere},
    {"cylinder", makeCylinder},
    {"cone", makeCone}};

std::unique_ptr<PrimitiveData> PrimitiveLoader::loadPrimitiveFromType(const std::string& type) {
    VertAndIndex vertAndIndex = primitiveMap[type]();
    recomputeNormals(vertAndIndex.first, vertAndIndex.second);
    return vertexIndex2PrimitiveData(vertAndIndex.first, vertAndIndex.second);
}