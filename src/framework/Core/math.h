#pragma once
#include <glm/glm.hpp>

namespace math {
    const float PI = 3.14159265359f;

    inline float     toDegrees(float radians) { return radians * 180.f / PI; }
    inline float     toRadians(float degree) { return degree * PI / 180.f; }
    inline glm::vec3 toRadians(const glm::vec3& angles) { return glm::vec3(toRadians(angles.x), toRadians(angles.y), toRadians(angles.z)); }

    inline glm::vec3 toDegrees(const glm::vec3& angles) { return glm::vec3(toDegrees(angles.x), toDegrees(angles.y), toDegrees(angles.z)); }
    glm::quat eulerYXZQuat(float yaw, float pitch, float roll);
    glm::quat eulerYXZQuat(const glm::vec3& eulerAngles);
}

