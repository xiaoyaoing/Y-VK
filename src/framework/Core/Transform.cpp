#include "Transform.h"

#include "imgui.h"
#include "math.h"

#define GLM_ENABLE_EXPERIMENTAL

#include "Common/Log.h"

#include <gtx/quaternion.hpp>
#include <gtx/rotate_vector.inl>
#include <gtx/transform.inl>

void LogMatrix(const glm::mat4& matrix) {
    LOGI("Matrix: col0 {} {} {} {} col1 {} {} {} {} col2 {} {} {} {} col3 {} {} {} {}", matrix[0][0], matrix[1][0], matrix[2][0], matrix[3][0], matrix[0][1], matrix[1][1], matrix[2][1], matrix[3][1], matrix[0][2], matrix[1][2], matrix[2][2], matrix[3][2], matrix[0][3], matrix[1][3], matrix[2][3], matrix[3][3]);
}

void Transform::setParent(const Transform* parent) {
    // m_position += parent->getPosition();
    // m_scale *= parent->getLocalScale();
    // m_rotation = parent->getRotation() * m_rotation;
    // updateCache();
    m_localToWorldMatrix = parent->getLocalToWorldMatrix() * m_localMatrix;
}
Transform::Transform(const BBox& bbox) {
    setBBox(bbox);
    m_lastFrameWorldBBox = m_worldBBox;
}

void Transform::onShowInEditor() {
    m_eulerAnglesWorld = math::toDegrees(m_eulerAnglesWorld);

    bool positionChanged = ImGui::DragFloat3("Position", &m_position[0], 0.05f);
    bool scaleChanged    = ImGui::DragFloat3("Scale", &m_scale[0], 0.05f);
    bool rotationChanged = ImGui::DragFloat3("Rotation", &m_eulerAnglesWorld[0], 0.5f, -360.0f, 360.0f);

    m_eulerAnglesWorld = math::toRadians(m_eulerAnglesWorld);

    if (positionChanged || scaleChanged || rotationChanged) {
        // This will also update the cache for position and scale
        setEulerAngles(m_eulerAnglesWorld);
    }
}

void Transform::lateUpdate() {
    m_changedSinceLastFrame = false;
    m_lastFrameWorldBBox    = m_worldBBox;
}
void Transform::setLocalToWorldMatrix(const glm::mat4& matrix) {
    m_localToWorldMatrix = matrix;
    m_worldToLocalMatrix = glm::inverse(matrix);

    m_position = glm::vec3(matrix[3]);
    m_rotation = glm::normalize(glm::quat_cast(matrix));
    m_scale    = getApproximateScale();

    m_changedSinceLastFrame = true;
    m_worldBBox             = m_originalBBox.toWorld(m_localToWorldMatrix);
}

glm::mat3 Transform::getLocalToWorldRotationMatrix() const {
    return glm::toMat3(getLocalToWorldRotation());
}

void Transform::setLocalPosition(const glm::vec3& position) {
    m_position = position;
    updateCache();
}

void Transform::setLocalRotation(const glm::quat& rotation) {
    m_rotation = rotation;
    updateCache();
    m_eulerAnglesWorld = getEulerAngles();
}

void Transform::setPosition(const glm::vec3& position) {
    m_position = position;
    updateCache();
}

void Transform::setRotation(const glm::quat& rotation) {
    m_rotation = rotation;
    updateCache();
    m_eulerAnglesWorld = getEulerAngles();
}
void Transform::setRotation(const glm::mat4& rotationMatrix) {
    m_rotation = glm::toQuat(rotationMatrix);
    auto t     = glm::toMat4(m_rotation);
    updateCache();
    m_eulerAnglesWorld = getEulerAngles();
}

glm::vec3 Transform::getApproximateScale() const {
    const glm::mat4& world = getLocalToWorldMatrix();

    return glm::vec3(glm::length(world[0]), glm::length(world[1]), glm::length(world[2]));
}

void Transform::setLocalScale(const glm::vec3& scale) {
    m_scale = scale;
    updateCache();
}

void Transform::lookAt(const glm::vec3& target, const glm::vec3& worldUp) {
    glm::mat3 rotation;

    rotation[2] = glm::normalize(target - getPosition());
    rotation[0] = glm::normalize(glm::cross(worldUp, rotation[2]));
    rotation[1] = glm::cross(rotation[2], rotation[0]);

    setRotation(glm::toQuat(rotation));
}

void Transform::setLocalEulerAngles(const glm::vec3& eulerAngles) {
    m_rotation = math::eulerYXZQuat(eulerAngles);
    updateCache();
    m_eulerAnglesWorld = getEulerAngles();
}

glm::vec3 Transform::getLocalEulerAngles() const { return eulerAngles(m_rotation); }

void Transform::setEulerAngles(const glm::vec3& eulerAngles) {
    glm::quat rotation = math::eulerYXZQuat(eulerAngles);
    m_rotation         = rotation;
    updateCache();
    m_eulerAnglesWorld = eulerAngles;
}

glm::vec3 Transform::getEulerAngles() const {
    return eulerAngles(getLocalToWorldRotation());
}

glm::vec3 Transform::transformPointToWorld(const glm::vec3& point) const {
    return glm::vec3(getLocalToWorldMatrix() * glm::vec4(point, 1.0f));
}

glm::vec3 Transform::transformVectorToWorld(const glm::vec3& vector) const {
    return glm::vec3(getLocalToWorldMatrix() * glm::vec4(vector, 0.0f));
}

glm::vec3 Transform::transformPointToLocal(const glm::vec3& point) const {
    return glm::vec3(getWorldToLocalMatrix() * glm::vec4(point, 1.0f));
}

glm::vec3 Transform::transformVectorToLocal(const glm::vec3& vector) const {
    return glm::vec3(getWorldToLocalMatrix() * glm::vec4(vector, 0.0f));
}

void Transform::setBBox(const BBox& bbox) noexcept {
    m_originalBBox          = bbox;
    m_worldBBox             = bbox.toWorld(m_localToWorldMatrix);
    m_changedSinceLastFrame = true;
    m_lastFrameWorldBBox    = m_worldBBox;
}

void Transform::updateCache() {
    m_localMatrix    = glm::translate(m_position) * glm::toMat4(m_rotation) * glm::scale(m_scale);
    m_localMatrixInv = glm::inverse(m_localMatrix);

    updateCacheHierarchy();
}

void Transform::updateCacheHierarchy() {

    m_localToWorldMatrix   = m_localMatrix;
    m_worldToLocalMatrix   = m_localMatrixInv;
    m_localToWorldRotation = m_rotation;
    m_worldToLocalRotation = glm::inverse(m_localToWorldRotation);

    m_changedSinceLastFrame = true;
    m_worldBBox             = m_originalBBox.toWorld(m_localToWorldMatrix);
}

glm::mat4 Transform::getLocalMatrix() const {
    return m_localMatrix;
}

glm::mat4 Transform::getLocalInverseMatrix() const {
    return m_localMatrixInv;
}

void Transform::pitch(float angle) noexcept {
    // Rotate up and look vector about the right vector.
    glm::mat3 rotation;

    rotation[0] = getRight();
    rotation[1] = glm::vec3(glm::rotate(angle, getRight()) * glm::vec4(getUp(), 0));
    rotation[2] = glm::vec3(glm::rotate(angle, getRight()) * glm::vec4(getForward(), 0));

    setRotation(glm::normalize(glm::toQuat(rotation)));
}

void Transform::rotateY(float angle) noexcept {
    // Rotate the basis vectors about the world y-axis.
    glm::mat3 rotation;
    rotation[0] = glm::rotateY(getRight(), angle);
    rotation[1] = glm::rotateY(getUp(), angle);
    rotation[2] = glm::rotateY(getForward(), angle);

    setRotation(glm::normalize(glm::toQuat(rotation)));
}

void Transform::roll(float angle) noexcept {
    // Rotate up and right vector about the forward vector.
    glm::mat3 rotation;

    rotation[0] = glm::vec3(glm::rotate(angle, getForward()) * glm::vec4(getRight(), 0));
    rotation[1] = glm::vec3(glm::rotate(angle, getForward()) * glm::vec4(getUp(), 0));
    rotation[2] = getForward();

    setRotation(glm::normalize(glm::toQuat(rotation)));
}
