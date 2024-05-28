#include "JsonUtil.h"
#include <fstream>
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
