enum class LIGHT_TYPE : uint8_t
{
    Directional = 0,
    Point = 1,
    Spot = 2,
    // Insert new light type here
    Max
};

struct LightProperties
{
    glm::vec3 direction{0.0f, 0.0f, -1.0f};

    glm::vec3 position;

    glm::vec3 color{1.0f, 1.0f, 1.0f};

    float intensity{1.0f};

    float range{0.0f};

    float inner_cone_angle{0.0f};

    float outer_cone_angle{0.0f};
};

struct Light
{
    LIGHT_TYPE type;
    LightProperties lightProperties;
};

struct alignas(16) LightUBO
{
    glm::vec4 color; // color.w represents light intensity
    glm::vec4 position; // position.w represents type of light
    glm::vec4 direction; // direction.w represents range
    glm::vec2 info;
    // (only used for spot lights) info.x represents light inner cone angle, info.y represents light outer cone angle
};

#define MAX_DEFERRED_LIGHT_COUNT 32

struct alignas(16) DeferredLights
{
    LightUBO directionalLights[MAX_DEFERRED_LIGHT_COUNT];
    LightUBO pointLights[MAX_DEFERRED_LIGHT_COUNT];
    LightUBO spotLights[MAX_DEFERRED_LIGHT_COUNT];
};


struct LightingState
{
    std::vector<Light> directionalLights;
    std::vector<Light> pointLights;
    std::vector<Light> spotLights;

    std::unique_ptr<Buffer> buffer{nullptr};
    void addLight(const Light& light);
};

inline void LightingState::addLight(const Light& light)
{
    switch (light.type)
    {
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
