#pragma once

#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>

class Camera {
    enum Mode {
        FIRST_PERSON,
        THIRD_PERSON
    };

public:
    Camera();

    ~Camera();


    struct {
        bool up{false};
        bool down{false};
        bool left{false};
        bool right{false};
    } keys;

    struct {
        bool left{false};
        bool middle{false};
        bool right{false};
    } mouseButtons;

    bool flipY{false};


    struct {
        glm::mat4 view;
        glm::mat4 perspective;
    } matrices;

    glm::vec4 viewPos{0};

    glm::vec3 position{0}, rotation{0};


    
    void updateViewMatrix();

    void update(float deltaTime);

    bool moving() const;

    void setPerspective(float fov, float aspect, float zNear, float zFar);

    void setRotation(glm::vec3 rotation);

    void setRotationByCamFront(glm::vec3 camFront);

    void setTranslation(glm::vec3 translation);

    void translate(const glm::vec3 &delta);

    void rotate(const glm::vec3 &delta);

    void setMoveSpeed(float moveSpeed);

    float getMoveSpeed() const;

    float rotationSpeed{1};

private:
    Mode mode{FIRST_PERSON};

    bool dirty{false};


    glm::vec3 cameraFront;

    float mMoveSpeed{1};

    float zNear{0.01}, zFar{10}, fov{45}, aspect{1};

    // bool rotating{false};
};
