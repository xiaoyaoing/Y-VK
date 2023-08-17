//
// Created by pc on 2023/8/17.
//

#include "FIleUtils.h"

namespace FileUtils {

    std::vector<uint8_t> read_binary_file(const std::string &filename, const uint32_t count) {
        std::vector<uint8_t> data;

        std::ifstream file;

        file.open(filename, std::ios::in | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filename);
        }

        uint64_t read_count = count;
        if (count == 0) {
            file.seekg(0, std::ios::end);
            read_count = static_cast<uint64_t>(file.tellg());
            file.seekg(0, std::ios::beg);
        }

        data.resize(static_cast<size_t>(read_count));
        file.read(reinterpret_cast<char *>(data.data()), read_count);
        file.close();

        return data;
    }


    std::vector<uint8_t> readShaderBinary(const std::string filename) {
        return read_binary_file(filename, 0);
    }

    std::string getFileExt(const std::string &filepath) {
        return filepath.substr(filepath.find('.') + 1);
    }
}
