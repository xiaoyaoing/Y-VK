//
// Created by pc on 2023/8/17.
//

#pragma once

#include <fstream>
#include <vector>

namespace FileUtils {
    std::string getFileExt(const std::string &filepath);

    std::vector<uint8_t> readShaderBinary(const std::string filename);
}

