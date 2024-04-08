/**
 * @file
 * @author JunPing Yuan
 * @brief
 * @version 0.1
 * @date 2023/3/3
 *
 * @copyright Copyright (c) 2022
 *
 */
#pragma once



#include <memory>
#include <map>
#include <vector>
#include <string>
#include <cstdarg>
#include <stdexcept>
#include <algorithm>
#include <glm/fwd.hpp>

#include "Common/Log.h"
#include "Common/DebugUtils.h"
#include "Common/VkUtils.h"
#include "Common/FIleUtils.h"
#include "Common/Enums.h"

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL

#include "vk_mem_alloc.h"

#define RUN_TIME_ERROR(error) throw std::runtime_error(error);
#define ASSERT(value, message) \
    if (!value)                \
        throw std::runtime_error(message);

#define DATA_SIZE(vec) (vec.size() * sizeof(vec[0]))

using uint32 = uint32_t;

template<class T>
inline uint32 toUint32(T t) {
    return uint32(t);
}

template<typename T>
using ptr = std::shared_ptr<T>;

template<class T, class Handle>
std::vector<Handle> getHandles(const std::vector<T&>& vec) {
    std::vector<Handle> handles;
    handles.reserve(vec.size());
    std::transform(vec.begin(), vec.end(), handles.begin(), [](T* t) { return t->getHandle(); });
    return handles;
}

template<class T>
std::vector<T> AllocateVector(size_t size) {
    std::vector<T> vec;
    vec.reserve(size);
    return vec;
}

template<class T>
using BindingMap = std::map<uint32_t, std::map<uint32_t, T>>;

template<typename T>
class Singleton {
public:
    static T& instance;

    static T& getInstance() {
        return instance;
    }

    Singleton(const Singleton&) = delete;

    Singleton& operator=(const Singleton&) = delete;

private:
    Singleton() {
    }
};

namespace Default {
    std::vector<VkClearValue> clearValues();
}

struct LoadStoreInfo {
    VkAttachmentLoadOp load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;

    VkAttachmentStoreOp store_op = VK_ATTACHMENT_STORE_OP_STORE;
};




