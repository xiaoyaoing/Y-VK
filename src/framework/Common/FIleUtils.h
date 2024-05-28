//
// Created by pc on 2023/8/17.
//

#pragma once

#include <fstream>
#include <vector>

namespace FileUtils {
    std::string getFileExt(const std::string &filepath);

    std::string getResourcePath();

    std::string getResourcePath(const std::string &path);

    std::string getShaderPath();

    std::string getShaderPath(const std::string &path);

    std::vector<uint8_t> readShaderBinary(const std::string filename);

    std::string getFileTimeStr(const std::string &path,const std::string & format = "%Y-%m-%d %H:%M:%S");

    bool fileExists(const std::string &path);

    template<typename T>
    static inline void streamRead(std::istream& in, T& dst) {
        in.read(reinterpret_cast<char*>(&dst), sizeof(T));
    }

    template<typename T>
    static inline T streamRead(std::istream& in) {
        T t;
        streamRead(in, t);
        return t;
    }

    template<typename T>
    static inline void streamRead(std::istream& in, std::vector<T>& dst) {
        in.read(reinterpret_cast<char*>(&dst[0]), dst.size() * sizeof(T));
    }

    template<typename T>
    static inline void streamRead(std::istream& in, T* dst, size_t n) {
        in.read(reinterpret_cast<char*>(dst), n * sizeof(T));
    }
}

