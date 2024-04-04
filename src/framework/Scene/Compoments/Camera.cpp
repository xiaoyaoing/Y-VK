#include <glm/ext/matrix_clip_space.hpp>
#include "Camera.h"

#include <glm/vec3.hpp>

Camera::Camera() {
}

Camera::~Camera() {
}

void Camera::update(float deltaTime) {
    if (moving()) {
        if (mode == Camera::FIRST_PERSON) {
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

bool Camera::moving() const {
    return keys.up || keys.down || keys.left || keys.right || mouseButtons.left || mouseButtons.right || mouseButtons.middle;
}

void Camera::updateViewMatrix() {
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

void Camera::setPerspective(float fov, float aspect, float znear, float zfar) {
    this->fov   = fov;
    this->zNear = znear;
    this->zFar  = zfar;

    //inverse znear and zfar
    matrices.perspective = glm::perspective(glm::radians(fov), aspect, zfar, znear);
    if (flipY) {
        matrices.perspective[1][1] *= -1.0f;
    }
}

void Camera::setRotation(glm::vec3 rotation) {
    this->rotation = rotation;
    updateViewMatrix();
}

void Camera::setTranslation(glm::vec3 translation) {
    this->position = translation;
    updateViewMatrix();
}

void Camera::setRotationByCamFront(glm::vec3 camFront) {
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

void Camera::translate(const glm::vec3& delta) {
    position += delta;
    updateViewMatrix();
}

void Camera::rotate(const glm::vec3& delta) {
    rotation += delta;
    updateViewMatrix();
}

void Camera::setMoveSpeed(float moveSpeed) {
    mMoveSpeed = moveSpeed;
}

float Camera::getMoveSpeed() const {
    return mMoveSpeed;
}