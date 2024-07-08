#pragma once
#include <glm.hpp>

namespace math {
    constexpr float PI          = 3.1415926536f;
    constexpr float PI_HALF     = PI*0.5f;
    constexpr float TWO_PI      = PI*2.0f;
    constexpr float FOUR_PI     = PI*4.0f;
    constexpr float INV_PI      = 1.0f/PI;
    constexpr float INV_TWO_PI  = 0.5f*INV_PI;
    constexpr float INV_FOUR_PI = 0.25f*INV_PI;
    constexpr float SQRT_PI     = 1.77245385091f;
    constexpr float INV_SQRT_PI = 1.0f/SQRT_PI;
    
    inline float     toDegrees(float radians) { return radians * 180.f / PI; }
    inline float     toRadians(float degree) { return degree * PI / 180.f; }
    inline glm::vec3 toRadians(const glm::vec3& angles) { return glm::vec3(toRadians(angles.x), toRadians(angles.y), toRadians(angles.z)); }

    inline glm::vec3 toDegrees(const glm::vec3& angles) { return glm::vec3(toDegrees(angles.x), toDegrees(angles.y), toDegrees(angles.z)); }
    glm::quat eulerYXZQuat(float yaw, float pitch, float roll);
    glm::quat eulerYXZQuat(const glm::vec3& eulerAngles);
    glm::quat eulerYZXQuat(const glm::vec3& eulerAngles);
}

