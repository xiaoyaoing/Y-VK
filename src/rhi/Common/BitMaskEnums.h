#pragma once
#include <type_traits>

template<typename Enum>
struct EnableBitMaskOperators : public std::false_type {};

template<typename Enum>
struct EnableIntegerOperators : public std::false_type {};

template<typename Enum, typename std::enable_if_t<std::is_enum<Enum>::value && EnableIntegerOperators<Enum>::value, int> = 0>
inline constexpr int operator+(Enum value) noexcept {
    return int(value);
}

template<typename Enum, typename std::enable_if_t<std::is_enum<Enum>::value && EnableIntegerOperators<Enum>::value, int> = 0>
inline constexpr bool operator==(Enum lhs, size_t rhs) noexcept {
    using underlying_t = std::underlying_type_t<Enum>;
    return underlying_t(lhs) == rhs;
}

template<typename Enum, typename std::enable_if_t<std::is_enum<Enum>::value && EnableIntegerOperators<Enum>::value, int> = 0>
inline constexpr bool operator==(size_t lhs, Enum rhs) noexcept {
    return rhs == lhs;
}

template<typename Enum, typename std::enable_if_t<std::is_enum<Enum>::value && EnableIntegerOperators<Enum>::value, int> = 0>
inline constexpr bool operator!=(Enum lhs, size_t rhs) noexcept {
    return !(rhs == lhs);
}

template<typename Enum, typename std::enable_if_t<std::is_enum<Enum>::value && EnableIntegerOperators<Enum>::value, int> = 0>
inline constexpr bool operator!=(size_t lhs, Enum rhs) noexcept {
    return rhs != lhs;
}

// ------------------------------------------------------------------------------------------------

template<typename Enum, typename std::enable_if_t<std::is_enum<Enum>::value && EnableBitMaskOperators<Enum>::value, int> = 0>
inline constexpr bool operator!(Enum rhs) noexcept {
    using underlying = std::underlying_type_t<Enum>;
    return underlying(rhs) == 0;
}

template<typename Enum, typename std::enable_if_t<std::is_enum<Enum>::value && EnableBitMaskOperators<Enum>::value, int> = 0>
inline constexpr Enum operator~(Enum rhs) noexcept {
    using underlying = std::underlying_type_t<Enum>;
    return Enum(~underlying(rhs));
}

template<typename Enum, typename std::enable_if_t<std::is_enum<Enum>::value && EnableBitMaskOperators<Enum>::value, int> = 0>
inline constexpr Enum operator|(Enum lhs, Enum rhs) noexcept {
    using underlying = std::underlying_type_t<Enum>;
    return Enum(underlying(lhs) | underlying(rhs));
}

template<typename Enum, typename std::enable_if_t<std::is_enum<Enum>::value && EnableBitMaskOperators<Enum>::value, int> = 0>
inline constexpr Enum operator&(Enum lhs, Enum rhs) noexcept {
    using underlying = std::underlying_type_t<Enum>;
    return Enum(underlying(lhs) & underlying(rhs));
}

template<typename Enum, typename std::enable_if_t<std::is_enum<Enum>::value && EnableBitMaskOperators<Enum>::value, int> = 0>
inline constexpr Enum operator^(Enum lhs, Enum rhs) noexcept {
    using underlying = std::underlying_type_t<Enum>;
    return Enum(underlying(lhs) ^ underlying(rhs));
}

template<typename Enum, typename std::enable_if_t<std::is_enum<Enum>::value && EnableBitMaskOperators<Enum>::value, int> = 0>
inline constexpr Enum operator|=(Enum& lhs, Enum rhs) noexcept {
    return lhs = lhs | rhs;
}

template<typename Enum, typename std::enable_if_t<std::is_enum<Enum>::value && EnableBitMaskOperators<Enum>::value, int> = 0>
inline constexpr Enum operator&=(Enum& lhs, Enum rhs) noexcept {
    return lhs = lhs & rhs;
}

template<typename Enum, typename std::enable_if_t<std::is_enum<Enum>::value && EnableBitMaskOperators<Enum>::value, int> = 0>
inline constexpr Enum operator^=(Enum& lhs, Enum rhs) noexcept {
    return lhs = lhs ^ rhs;
}

template<typename Enum, typename std::enable_if_t<std::is_enum<Enum>::value && EnableBitMaskOperators<Enum>::value, int> = 0>
inline constexpr bool none(Enum lhs) noexcept {
    return !lhs;
}

template<typename Enum, typename std::enable_if_t<std::is_enum<Enum>::value && EnableBitMaskOperators<Enum>::value, int> = 0>
inline constexpr bool any(Enum lhs) noexcept {
    return !none(lhs);
}