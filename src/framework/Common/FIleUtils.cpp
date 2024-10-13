//
// Created by pc on 2023/8/17.
//

#include "FIleUtils.h"

#include <chrono>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctime>
#include <filesystem>

namespace FileUtils {

    template<typename... Args>
    std::string stringFormat(const char* format, Args... args) {
        int size_s = std::snprintf(nullptr, 0, format, args...) + 1;
        if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
        auto                    size = static_cast<size_t>(size_s);
        std::unique_ptr<char[]> buf(new char[size]);
        std::snprintf(buf.get(), size, format, args...);
        return std::string(buf.get(), buf.get() + size - 1);
    }

    std::vector<uint8_t> read_binary_file(const std::string& filename, const uint32_t count) {
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
        file.read(reinterpret_cast<char*>(data.data()), read_count);
        file.close();

        return data;
    }

    std::string FileUtils::getFilePath(const std::string& path, const std::string& suffix, bool overwrite) {
        std::string destPath = path + "." + suffix;
        if (overwrite)
            return std::move(destPath);
        int count = 1;
        while (fileExists(destPath)) {
            destPath = stringFormat("%s%d.%s", path.c_str(), count++, suffix.c_str());
        }
        return std::move(destPath);
    }

    std::string getFileTimeStr(const std::string& path, const std::string& format) {
        const auto fileTime   = std::filesystem::last_write_time(path);
        const auto systemTime = std::chrono::clock_cast<std::chrono::system_clock>(fileTime);
        const auto time       = std::chrono::system_clock::to_time_t(systemTime);

        struct tm stime;
        localtime_s(&stime, &time);

        char tmp[32] = {NULL};
        strftime(tmp, sizeof(tmp), format.c_str(), &stime);

        return tmp;
    }
    bool fileExists(const std::string& path) {
        struct stat buffer;
        return (stat(path.c_str(), &buffer) == 0);
    }

    std::vector<uint8_t> readShaderBinary(const std::string filename) {
        return read_binary_file(filename, 0);
    }


    std::string getFileExt(const std::string& filepath) {
        return filepath.substr(filepath.find('.') + 1);
    }

    std::string getResourcePath() {
#ifdef VK_RESOURCES_DIR
        return VK_RESOURCES_DIR;
#else
        return "./../resources";
#endif
    }

    std::string getResourcePath(const std::string& path) {
        return getResourcePath() + path;
    }

    std::string getShaderPath(const std::string& path) {
        return getShaderPath() + path;
    }

    std::string getShaderPath() {
#ifdef VK_SHADERS_DIR
        return VK_SHADERS_DIR;
#else
        return "./../shaders";
#endif
    }

}// namespace FileUtils
