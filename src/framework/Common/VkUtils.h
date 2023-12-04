
#pragma once
template <class T>
constexpr T alignUp(T x, size_t a) noexcept {
    return T((x + (T(a) - 1)) & ~T(a - 1));
}
