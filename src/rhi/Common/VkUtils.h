
#pragma once
#include <vector>

template <class T>
constexpr T alignUp(T x, size_t a) noexcept {
    return T((x + (T(a) - 1)) & ~T(a - 1));
}

template<class T>
std::vector<uint8_t> toBytes(const T& value) noexcept {
    return std::vector<uint8_t>(reinterpret_cast<const uint8_t*>(&value), reinterpret_cast<const uint8_t*>(&value) + sizeof(T));
}
