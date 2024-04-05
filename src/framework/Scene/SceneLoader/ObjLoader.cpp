#include "ObjLoader.hpp"

template<glm::length_t size>
glm::vec<size, float, glm::defaultp> loadVector(const char* s) {
    std::istringstream                   ss(s);
    glm::vec<size, float, glm::defaultp> result;
    for (unsigned i = 0; i < size && !ss.eof() && !ss.fail(); ++i)
        ss >> result[i];
    return result;
}

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
//     auto iter = _indices.find(ivec3(pos, normal, uv));
//     if (iter != _indices.end()) {
//         return iter->second;
//     } else {
//         vec3 p(0.0f), n(0.0f, 1.0f, 0.0f);
//         vec2 u(0.0f);
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
//         _indices.insert(std::make_pair(ivec3(pos, normal, uv), index));
//         return index;
//     }
// }

PrimitiveData PrimitiveLoader::loadPrimitive(const std::string& path) {
    return {};
}