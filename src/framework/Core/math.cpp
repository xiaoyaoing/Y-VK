#include "math.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/quaternion.hpp>
#include <gtx/transform.inl>

#include <gtx/rotate_vector.inl>

namespace math {

    glm::quat
    eulerYXZQuat(float yaw, float pitch, float roll) {
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
    
    glm::quat fromXAxis(float angle) {
        return glm::quat(sinf(angle / 2), 0, 0, cosf(angle / 2));
    }

    glm::quat fromYAxis(float angle) {
        return glm::quat(0, sinf(angle / 2), 0, cosf(angle / 2));
    }

    glm::quat fromZAxis(float angle) {
        return glm::quat(0, 0, sinf(angle / 2), cosf(angle / 2));
    }
    

    glm::quat eulerYXZQuat(const glm::vec3& eulerAngles) { return eulerYXZQuat(eulerAngles.y, eulerAngles.x, eulerAngles.z); }

    glm::quat eulerYZXQuat(const glm::vec3& eulerAngles) {
        return fromXAxis(eulerAngles.x) * fromZAxis(eulerAngles.z) * fromYAxis(eulerAngles.y);
    }
}// namespace math
