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
#pragma  once

#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>
#include <memory>
#include <vector>
#include <string>
#include <cstdarg>
#include <stdexcept>

#include <Common/Log.h>
#include <Common/String.h>

#define  RUN_TIME_ERROR(error) throw std::runtime_error(error);
#define  ASSERT(value, message) if(!value)throw std::runtime_error(message);
#define  VK_VERIFY_RESULT(VkFunction) {const VkResult scopedResult = VkFunction; \
                                    if(scopedResult!=VK_SUCCESS)               \
                                    throw std::runtime_error(string_format("VKResult=%d,Function=%s,File=%s,Line=%d", scopedResult, #VkFunction, __FILE__, __LINE__));\
                                    }
#define  DATA_SIZE(vec) (vec.size() * sizeof(vec[0]))

typedef uint32_t uint32;

template<typename T>
using ptr = std::shared_ptr<T>;


template<class T, class Handle>
std::vector<Handle> getHandles(const std::vector<ptr<T>> &vec) {
    std::vector<Handle> handles;
    handles.reserve(vec.size());
    for (const auto &obj: vec)
        handles.push_back(obj->getHandle());
    return handles;
}

template<class T>
std::vector<T> AllocateVector(size_t size) {
    std::vector<T> vec;
    vec.reserve(size);
    return vec;
}

template<typename T>
class Singleton {
public:
    static T instance;

    static T &getInstance() {
        // 使用局部静态变量确保在第一次调用时初始化，且线程安全
        return instance;
    }

    // 禁止拷贝构造和赋值运算符
    Singleton(const Singleton &) = delete;

    Singleton &operator=(const Singleton &) = delete;

private:
    Singleton() {} // 私有构造函数，确保只能通过 getInstance() 获取实例
};

namespace Default {
    std::vector<VkClearValue> clearValues();
}