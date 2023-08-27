//
// Created by pc on 2023/8/27.
//

#pragma once

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "API_VK.h"
#include "Mesh.h"

struct Skin;

struct Material {
    Texture *baseColor;

};

struct Node {
    Node *parent;
    uint32_t index;
    std::vector<Node *> children;
    glm::mat4 matrix;
    std::string name;
    Mesh *mesh;
    Skin *skin;
    int32_t skinIndex = -1;
    glm::vec3 translation{};
    glm::vec3 scale{1.0f};
    glm::quat rotation{};

    glm::mat4 localMatrix();

    glm::mat4 getMatrix();

    void update();

    ~Node();
};

struct Skin {
    std::string name;
    Node *skeletonRoot = nullptr;
    std::vector<glm::mat4> inverseBindMatrices;
    std::vector<Node *> joints;
};

class gltfloader {

};

struct AnimationChannel {
    enum PathType {
        TRANSLATION, ROTATION, SCALE
    };
    PathType path;
    Node *node;
    uint32_t samplerIndex;
};

/*
    glTF animation sampler
*/
struct AnimationSampler {
    enum InterpolationType {
        LINEAR, STEP, CUBICSPLINE
    };
    InterpolationType interpolation;
    std::vector<float> inputs;
    std::vector<glm::vec4> outputsVec4;
};

/*
    glTF animation
*/
struct Animation {
    std::string name;
    std::vector<AnimationSampler> samplers;
    std::vector<AnimationChannel> channels;
    float start = std::numeric_limits<float>::max();
    float end = std::numeric_limits<float>::min();
};

struct Model {
    Device *device;
    Mesh *mesh;
    std::vector<Node *> nodes;
    std::vector<Node *> linearNodes;

    std::vector<Skin *> skins;

    std::vector<Texture> textures;
    std::vector<Material> materials;
    std::vector<Animation> animations;

    void loadFromFile(const std::string &path);
};


