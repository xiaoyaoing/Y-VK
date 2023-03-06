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

#include "vulkan/vulkan_core.h"
#include "ext/VulkanMemoryAllocator/include/vk_mem_alloc.h"
#include <vector>

#define  RUN_TIME_ERROR(error) throw std::runtime_error(error);
#define  ASSERT(value) if(!value) throw std::runtime_error("Error")
#define  ASSERTEQ(a, b) ASSERT(a == b)
#define  DATASIZE(vec) vec.size() * sizeof(vec[0])

template<typename T>
using ptr = std::shared_ptr<T>;

//template<class T>
//inline  uint32_t  getVectorDataSize(const std::vector<T> & vec){
//    return vec.size() * sizeof(vec[0]);
//}

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