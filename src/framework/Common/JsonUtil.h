#pragma once
#include <fwd.hpp>
#include <vec3.hpp>
#include <vec2.hpp>
#include <nlohmann/json.hpp>

using Json = nlohmann::json;


template<typename T>
T GetOptional(const Json& j, const std::string& key, const T& defaultValue) {
    if (j.contains(key)) {
        return j[key];
    }
    return defaultValue;
}

template<class T>
inline bool ContainsAndGet(const Json& j, std::string field, T& value) {
    if (j.find(field) != j.end()) {
        value = j.at(field).get<T>();
        return true;
    }
    return false;
}

class JsonUtil {
public:
    static Json fromFile(const std::string& path);
    static void toFile(const std::string& path, const Json& json);
};

namespace glm {
    void from_json(const Json& j, vec3& v);
    void from_json(const Json& j, ivec3& v);

    void from_json(const Json& j, vec2& v);
    void from_json(const Json& j, quat& v);
}// 