#define GLM_ENABLE_EXPERIMENTAL

#include <glm/ext/matrix_clip_space.hpp>
#include "Camera.h"

#include "imgui.h"
#include "Core/math.h"

#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>
Camera0::Camera0() {
}

Camera0::~Camera0() {
}

void Camera0::update(float deltaTime) {
    if (moving()) {
        if (mode == Camera0::FIRST_PERSON) {
            glm::vec3 camFront;
            camFront.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
            camFront.y = sin(glm::radians(rotation.x));
            camFront.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
            camFront   = glm::normalize(camFront);

            mMoveSpeed += deltaTime;
            float moveDistance = deltaTime * mMoveSpeed;

            if (keys.up) {
                position += camFront * moveDistance;
            }
            if (keys.down) {
                position -= camFront * moveDistance;
            }
            if (keys.left)
                position -= glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveDistance;
            if (keys.right)
                position += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveDistance;
            updateViewMatrix();
        }
    } else {
        mMoveSpeed = 1.f;
    }
}

bool Camera0::moving() const {
    return keys.up || keys.down || keys.left || keys.right || mouseButtons.left || mouseButtons.right || mouseButtons.middle;
}

void Camera0::updateViewMatrix() {
    //    if (dirty) {
    auto      rotM = glm::mat4(1.0f);
    glm::mat4 transM{1};

    rotM = glm::rotate(rotM, glm::radians(rotation.x * (flipY ? -1.0f : 1.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
    rotM = glm::rotate(rotM, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    rotM = glm::rotate(rotM, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    glm::vec3 translation = position;
    if (flipY)
        translation.y *= -1.0f;
    transM = glm::translate(transM, translation);

    if (mode == FIRST_PERSON)
        matrices.view = rotM * transM;
    else if (mode == THIRD_PERSON)
        matrices.view = transM * rotM;
}

void Camera0::setPerspective(float fov, float aspect, float znear, float zfar) {
    this->fov   = fov;
    this->zNear = znear;
    this->zFar  = zfar;

    //inverse znear and zfar
    matrices.perspective = glm::perspective(glm::radians(fov), aspect, zfar, znear);
    if (flipY) {
        matrices.perspective[1][1] *= -1.0f;
    }
}

void Camera0::setRotation(glm::vec3 rotation) {
    this->rotation = rotation;
    updateViewMatrix();
}

void Camera0::setTranslation(glm::vec3 translation) {
    this->position = translation;
    updateViewMatrix();
}

void Camera0::setRotationByCamFront(glm::vec3 camFront) {
    ////    camFront.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
    ////    camFront.y = sin(glm::radians(rotation.x));
    ////    camFront.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
    ////    rotation.x =
    //    rotation.x = glm::degrees(std::asin(camFront.y));
    //    rotation.y = glm::degrees(std::acos(camFront.z / cos(glm::radians(rotation.x))));
    ////    rotation.x = 0;
    ////    rotation.y = 0;
    //    rotation.z = 0;
    //    updateViewMatrix();
}

void Camera0::translate(const glm::vec3& delta) {
    position += delta;
    updateViewMatrix();
}

void Camera0::rotate(const glm::vec3& delta) {
    rotation += delta;
    updateViewMatrix();
}

void Camera0::setMoveSpeed(float moveSpeed) {
    mMoveSpeed = moveSpeed;
}

float Camera0::getMoveSpeed() const {
    return mMoveSpeed;
}

void Frustum::transform(const glm::mat4& matrix) {
    for (auto& p : points)
        p = glm::vec3(matrix * glm::vec4(p, 1.0f));
}

BBox Frustum::getBBox() const {
    BBox bbox;

    for (auto& p : points)
        bbox.unite(p);

    return bbox;
}
Camera::Camera() {
    m_transform = std::make_unique<Transform>();
}

// Camera::Camera() {}
Camera::~Camera() {
}

void Camera::update(float deltaTime) {
    if (moving() || firstUpdate) {
        {
            mMoveSpeed += deltaTime;
            float moveDistance = deltaTime * mMoveSpeed;

            if (keys.up) {
                walk(moveDistance);
            }
            if (keys.down) {
                walk(-moveDistance);
            }
            if (keys.left)
                strafe(-moveDistance);
            if (keys.right)
                strafe(moveDistance);
            updateViewMatrix();
            firstUpdate = false;
        }
    } else {
        // mMoveSpeed = 1.f;
    }
}

void Camera::onShowInEditor() {
    glm::vec3 pos = getPosition();
    ImGui::Text("Camera Position: %.2f %.2f %.2f", pos.x, pos.y, pos.z);
    glm::quat rotat = getTransform()->getRotation();
    ImGui::Text("Camera Rotation: %.2f %.2f %.2f %.2f", rotat.x, rotat.y, rotat.z, rotat.w);
    ImGui::PopItemWidth();
    ImGui::NextColumn();
    ImGui::InputFloat("Camera Move Speed", &mMoveSpeed);
    
    const char* mode[]{
        "Perspective",
        "Orthographic"};

    int curItem = m_perspective ? 0 : 1;
    ImGui::Combo("Mode", &curItem, mode, 2);
    m_perspective = curItem == 0;

    ImGui::DragFloat("Near Plane", &m_nearZ, 0.01f, 0.01f, 1000.0f);
    ImGui::DragFloat("Far Plane", &m_farZ, 0.01f, 0.0f, 1000.0f);
    ImGui::Checkbox("Flip Y", &flipYTemp);
    if (flipYTemp != flipY) {
        flipY = flipYTemp;
        updateProjMatrix();
    }
}

void Camera::setOrthographic(float screenWidth, float screenHeight, float zn, float zf) noexcept {
    m_screenWidth  = screenWidth;
    m_screenHeight = screenHeight;
    m_nearZ        = zn;
    m_farZ         = zf;
    m_perspective  = false;

    setViewport(0.0f, 0.0f, screenWidth, screenHeight);
}

void Camera::setPerspective(float fovY, float screenWidth, float screenHeight, float zn, float zf) noexcept {
    m_screenWidth  = screenWidth;
    m_screenHeight = screenHeight;
    m_fovY         = fovY;
    m_aspect       = screenWidth / screenHeight;
    m_nearZ        = zn;
    m_farZ         = zf;
    m_perspective  = true;

    setViewport(0.0f, 0.0f, screenWidth, screenHeight);
}

void Camera::setViewport(float x, float y, float width, float height) noexcept {
    m_viewport           = Rect(x, y, x + width, y + height);
    m_normalizedViewport = Rect(x / m_screenWidth, y / m_screenHeight, (x + width) / m_screenWidth, (y + height) / m_screenHeight);
    m_aspect             = m_viewport.width() / m_viewport.height();

    float nz = useInverseDepth ? m_farZ : m_nearZ;
    float fz = useInverseDepth ? m_nearZ : m_farZ;

    if (m_perspective)
        m_proj = glm::perspective(glm::radians(m_fovY), m_aspect, nz, fz);
    else
        m_proj = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, nz, fz);

    if (flipY)
        m_proj[1][1] *= -1.0f;

    m_projInv = glm::inverse(m_proj);
}

void Camera::resize(float screenWidth, float screenHeight) {
    m_screenWidth  = screenWidth;
    m_screenHeight = screenHeight;
    setViewport(m_normalizedViewport.minX() * m_screenWidth, m_normalizedViewport.minY() * m_screenHeight, m_normalizedViewport.width() * m_screenWidth, m_normalizedViewport.height() * m_screenHeight);
}

void Camera::updateViewMatrix() noexcept {
    auto transform = m_transform.get();

    glm::mat3 rotation = glm::toMat3(transform->getRotation());
    glm::vec3 pos      = transform->getPosition();
    // if (flipY)
    //     pos.y *= -1.0f;

    glm::vec3& right = rotation[0];
    glm::vec3& up    = rotation[1];
    glm::vec3& look  = rotation[2];

    float x = -glm::dot(pos, right);
    float y = -glm::dot(pos, up);
    float z = -glm::dot(pos, look);

    m_view[0][0] = right.x;
    m_view[1][0] = right.y;
    m_view[2][0] = right.z;
    m_view[3][0] = x;

    m_view[0][1] = up.x;
    m_view[1][1] = up.y;
    m_view[2][1] = up.z;
    m_view[3][1] = y;

    m_view[0][2] = look.x;
    m_view[1][2] = look.y;
    m_view[2][2] = look.z;
    m_view[3][2] = z;

    m_view[0][3] = 0.0f;
    m_view[1][3] = 0.0f;
    m_view[2][3] = 0.0f;
    m_view[3][3] = 1.0f;

    m_viewProj    = m_proj * m_view;
    m_viewInv     = glm::inverse(m_view);
    m_viewProjInv = m_viewInv * m_projInv;
}
void Camera::updateProjMatrix() noexcept {
    float nz = useInverseDepth ? m_farZ : m_nearZ;
    float fz = useInverseDepth ? m_nearZ : m_farZ;

    if (m_perspective)
        // use inverse zNear and zFar
        m_proj = glm::perspective(glm::radians(m_fovY), m_aspect, nz, fz);
    else
        m_proj = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, nz, fz);
    if (flipY)
        m_proj[1][1] *= -1.0f;
    m_projInv = glm::inverse(m_proj);
    updateViewMatrix();
}

glm::vec3 Camera::screenToWorldPoint(const glm::vec3& p) const noexcept {
    // Convert to NDC space
    glm::vec4 point(screenToNDC(p), 1.f);

    // Convert to world space
    point = m_viewProjInv * point;
    point /= point.w;

    return glm::vec3(point);
}

glm::vec3 Camera::screenToNDC(const glm::vec3& p) const noexcept {
    return glm::vec3(
        p.x / m_screenWidth * 2.0f - 1.0f,
        p.y / m_screenHeight * 2.0f - 1.0f,
        (p.z - m_nearZ) / (m_farZ - m_nearZ) * 2.0f - 1.0f);
}

glm::vec3 Camera::worldToScreenPoint(const glm::vec3& worldPoint) const noexcept {
    glm::vec4 ndc = m_viewProj * glm::vec4(worldPoint, 1.0f);
    ndc /= ndc.w;
    return glm::vec3((ndc.x * 0.5f + 0.5f) * m_screenWidth,
                     (ndc.y * 0.5f + 0.5f) * m_screenHeight,
                     (ndc.z + 1.f) * (m_farZ - m_nearZ) * 0.5f + m_nearZ);
}

glm::vec3 Camera::screenToViewportPoint(const glm::vec3& p) const noexcept { return glm::vec3(p.x - m_viewport.minX(), p.y - m_viewport.minY(), p.z); }

glm::vec3 Camera::viewportToScreenPoint(const glm::vec3& p) const noexcept { return glm::vec3(p.x + m_viewport.minX(), p.y + m_viewport.minY(), p.z); }

glm::vec3 Camera::viewportToNDC(const glm::vec3& p) const noexcept {
    return glm::vec3(
        p.x / m_viewport.width() * 2.0f - 1.0f,
        p.y / m_viewport.height() * 2.0f - 1.0f,
        (p.z - m_nearZ) / (m_farZ - m_nearZ) * 2.0f - 1.0f);
}

glm::vec3 Camera::viewportToWorldPoint(const glm::vec3& p) const noexcept {
    // Convert to NDC space
    glm::vec4 point(viewportToNDC(p), 1.f);

    // Convert to world space
    point = m_viewProjInv * point;
    point /= point.w;

    return glm::vec3(point);
}

glm::vec3 Camera::worldToViewportPoint(const glm::vec3& p) const noexcept {
    glm::vec4 ndc = m_viewProj * glm::vec4(p, 1.0f);
    ndc /= ndc.w;
    return glm::vec3((ndc.x * 0.5f + 0.5f) * m_viewport.width(),
                     (0.5f + ndc.y * 0.5f) * m_viewport.height(),
                     (ndc.z + 1.f) * (m_farZ - m_nearZ) * 0.5f + m_nearZ);
}

Ray Camera::screenPointToRay(const glm::vec3& p) const noexcept {
    // Convert to NDC space
    glm::vec3 ndcP = screenToNDC(p);
    glm::vec4 start(ndcP, 1.f);
    glm::vec4 end(ndcP.x, ndcP.y, 1.0f, 1.f);

    // Convert to world space
    start = m_viewProjInv * start;
    start /= start.w;

    end = m_viewProjInv * end;
    end /= end.w;

    return {};
    // return Ray(glm::vec3(start), glm::normalize(glm::vec3(end - start)));
}

Ray Camera::viewportPointToRay(const glm::vec3& p) const noexcept {
    // Convert to NDC space
    glm::vec3 ndcP = viewportToNDC(p);
    glm::vec4 start(ndcP, 1.f);
    glm::vec4 end(ndcP.x, ndcP.y, 1.0f, 1.f);

    // Convert to world space
    start = m_viewProjInv * start;
    start /= start.w;

    end = m_viewProjInv * end;
    end /= end.w;
    return {};

    //return Ray(glm::vec3(start), glm::normalize(glm::vec3(end - start)));
}

Frustum Camera::getFrustum() const {
    float nz = useInverseDepth ? m_farZ : m_nearZ;
    float fz = useInverseDepth ? m_nearZ : m_farZ;

    return getFrustum(nz, fz);
}

Frustum Camera::getFrustum(float nearZ, float farZ) const {
    Frustum frustum;

    float thHFOV = tanf(math::toRadians(getHorizontalFOV() * 0.5f));
    float thVFOV = tanf(math::toRadians(m_fovY * 0.5f));

    // Compute the frustum in view space
    float xNear = nearZ * thHFOV;
    float yNear = nearZ * thVFOV;
    float xFar  = farZ * thHFOV;
    float yFar  = farZ * thVFOV;

    // Transform to world space
    frustum.data.nearBottomLeft  = glm::vec3(m_viewInv * glm::vec4(-xNear, -yNear, nearZ, 1.0f));
    frustum.data.nearBottomRight = glm::vec3(m_viewInv * glm::vec4(xNear, -yNear, nearZ, 1.0f));
    frustum.data.nearTopLeft     = glm::vec3(m_viewInv * glm::vec4(-xNear, yNear, nearZ, 1.0f));
    frustum.data.nearTopRight    = glm::vec3(m_viewInv * glm::vec4(xNear, yNear, nearZ, 1.0f));

    frustum.data.farBottomLeft  = glm::vec3(m_viewInv * glm::vec4(-xFar, -yFar, farZ, 1.0f));
    frustum.data.farBottomRight = glm::vec3(m_viewInv * glm::vec4(xFar, -yFar, farZ, 1.0f));
    frustum.data.farTopLeft     = glm::vec3(m_viewInv * glm::vec4(-xFar, yFar, farZ, 1.0f));
    frustum.data.farTopRight    = glm::vec3(m_viewInv * glm::vec4(xFar, yFar, farZ, 1.0f));

    /*
    glm::vec4 nearBottomLeft  = m_viewProjInv * glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f);
    glm::vec4 nearBottomRight = m_viewProjInv * glm::vec4(1.0f, -1.0f, -1.0f, 1.0f);
    glm::vec4 nearTopLeft	  = m_viewProjInv * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);
    glm::vec4 nearTopRight	  = m_viewProjInv * glm::vec4(1.0f, 1.0f, -1.0f, 1.0f);

    glm::vec4 farBottomLeft	  = m_viewProjInv * glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f);
    glm::vec4 farBottomRight  = m_viewProjInv * glm::vec4(1.0f, -1.0f, 1.0f, 1.0f);
    glm::vec4 farTopLeft	  = m_viewProjInv * glm::vec4(-1.0f, 1.0f, 1.0f, 1.0f);
    glm::vec4 farTopRight	  = m_viewProjInv * glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

    frustum.nearBottomLeft  = glm::vec3(nearBottomLeft / nearBottomLeft.w);
    frustum.nearBottomRight = glm::vec3(nearBottomRight / nearBottomRight.w);
    frustum.nearTopLeft     = glm::vec3(nearTopLeft / nearTopLeft.w);
    frustum.nearTopRight    = glm::vec3(nearTopRight / nearTopRight.w);
                                        
    frustum.farBottomLeft   = glm::vec3(farBottomLeft / farBottomLeft.w);
    frustum.farBottomRight	= glm::vec3(farBottomRight / farBottomRight.w);
    frustum.farTopLeft		= glm::vec3(farTopLeft / farTopLeft.w);
    frustum.farTopRight		= glm::vec3(farTopRight / farTopRight.w);
    */

    return frustum;
}

bool Camera::moving() const {
    return keys.up || keys.down || keys.left || keys.right || mouseButtons.left || mouseButtons.right || mouseButtons.middle;
}
void Camera::setPerspective(float fov, float aspect, float zNear, float zFar) {
    m_fovY        = fov;
    m_aspect      = aspect;
    m_nearZ       = zNear;
    m_farZ        = zFar;
    m_perspective = true;

    updateProjMatrix();
}
void Camera::setRotation(glm::vec3 rotation) {
    m_transform->setRotation(math::eulerYZXQuat(rotation));
    updateViewMatrix();
}
void Camera::setRotation(glm::quat rotation) {
    m_transform->setRotation(rotation);
    updateViewMatrix();
}
void Camera::setMoveSpeed(float moveSpeed) {
    mMoveSpeed = moveSpeed;
}
float Camera::getMoveSpeed() const {
    return mMoveSpeed;
}
