#include "JsonUtil.h"
#include <fstream>
#include <detail/type_quat.hpp>
Json JsonUtil::fromFile(const std::string& path) {
    Json json;
    std::ifstream file(path);
    file >> json;
    return json;
}

void JsonUtil::toFile(const std::string& path, const Json& json) {
    std::ofstream file(path);
    file << json;
    file.close();
}

namespace glm {
    void from_json(const Json& j, vec3& v) {
        if (!j.is_array()) {
            v = vec3(j.get<float>());
            return;
        }
        v.x = j.at(0).get<float>();
        v.y = j.at(1).get<float>();
        v.z = j.at(2).get<float>();
    }

    void from_json(const Json& j, vec2& v) {
        if (!j.is_array()) {
            v = vec2(j.get<float>());
            return;
        }
        v.x = j.at(0).get<float>();
        v.y = j.at(1).get<float>();
    }

    void from_json(const Json& j, quat& v)
    {
        v.x = j.at(0).get<float>();
        v.y = j.at(1).get<float>();
        v.z = j.at(2).get<float>();
        v.w = j.at(3).get<float>();
    }
}// namespace glm