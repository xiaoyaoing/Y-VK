#include "math.h"
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.inl>
#include <glm/gtx/rotate_vector.inl>
glm::quat eulerYXZQuat(float yaw, float pitch, float roll)
{
    float cy = cosf(yaw * 0.5f);
    float sy = sinf(yaw * 0.5f);

    float cp = cosf(pitch * 0.5f);
    float sp = sinf(pitch * 0.5f);

    float cr = cosf(roll * 0.5f);
    float sr = sinf(roll * 0.5f);

    return glm::quat(cy * cp * cr + sy * sp * sr,
                     cy * sp * cr + sy * cp * sr,
                     sy * cp * cr - cy * sp * sr,
                     cy * cp * sr - sy * sp * cr);
}


glm::quat eulerYXZQuat(const glm::vec3& eulerAngles) { return eulerYXZQuat(eulerAngles.y, eulerAngles.x, eulerAngles.z); }