#pragma once

enum class LIGHT_TYPE : uint8_t {
    Directional = 0,
    Point       = 1,
    Spot        = 2,
    Area        = 3,
    Sky         = 4,
    // Insert new light type here
    Max
};

struct LightProperties {
    glm::vec3 direction{0.0f, 0.0f, -1.0f};

    glm::vec3 position;

    glm::vec3 color{1.0f, 1.0f, 1.0f};
    glm::vec3 power{0, 0, 0};

    float intensity{1.0f};

    float range{0.0f};

    float inner_cone_angle{0.0f};

    float outer_cone_angle{0.0f};

    int shadow_index{-1};

    uint32_t prim_index{0};
    uint32_t texture_index{0};
    glm::mat4 world_matrix{1.0f};
    
};

struct SgLight {
    LIGHT_TYPE      type;
    LightProperties lightProperties;
};

struct alignas(16) LightUib {
    glm::vec4 color{0};    // color.w represents light intensity
    glm::vec4 position{0}; // position.w represents type of light
    glm::vec4 direction{0};// direction.w represents range
    glm::vec4 info{0};
    // (only used for spot lights) info.x represents light inner cone angle, info.y represents light outer cone angle
};

#define MAX_DEFERRED_LIGHT_COUNT 32

struct alignas(16) DeferredLights {
    LightUib directionalLights[MAX_DEFERRED_LIGHT_COUNT];
    LightUib pointLights[MAX_DEFERRED_LIGHT_COUNT];
    LightUib spotLights[MAX_DEFERRED_LIGHT_COUNT];
};

struct LightingState {
    std::vector<SgLight> directionalLights;
    std::vector<SgLight> pointLights;
    std::vector<SgLight> spotLights;

    std::unique_ptr<Buffer> buffer{nullptr};
    void                    addLight(const SgLight& light);
};

inline void LightingState::addLight(const SgLight& light) {
    switch (light.type) {
        case LIGHT_TYPE::Directional:
            directionalLights.push_back(light);
            break;
        case LIGHT_TYPE::Point:
            pointLights.push_back(light);
            break;
        case LIGHT_TYPE::Spot:
            spotLights.push_back(light);
            break;
        case LIGHT_TYPE::Max:
        default:
            LOGE("Light type not supported")
    }
}

//todo 正确封装light 现在太丑了 
class Light {
    
};