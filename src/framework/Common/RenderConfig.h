#pragma once
#include "Common/JsonUtil.h"
#include "Scene/SceneLoader/SceneLoadingConfig.h"

#include <vec3.hpp>

struct DDGIConfig {
    float hysteresis = 0.98f;
    int rays_per_probe = 256;
    float depth_sharpness = 50.0f;
    float normal_bias = 0.1f;
    float view_bias = 0.1f;
    float backface_ratio = 0.1f;
    float min_frontface_dist = 0.1f;
    float max_distance;
    glm::ivec3 probe_counts = glm::ivec3(16,16,16);
    glm::vec3 probe_start_position;
    int irradiance_texel_count  = 8;
    int distance_texel_count    = 16;
    int inner_irradiance_texel_count = 6;
    int inner_distance_texel_count = 14;
    bool use_rt_gbuffer;
    bool debug_ddgi = true;
};

struct PathTracingConfig {
    int min_depth = 1;
    int max_depth = 5;
    bool sample_bsdf = true;
    bool sample_light = true;
};

enum EIntegraotrType {
    ePathTracing,
    eDDGI,
    eRestirDI,
};
 

std::string to_string(EIntegraotrType type);


class RenderConfig {
public:
    RenderConfig() = default;
    DDGIConfig getDDGIConfig() const;
    PathTracingConfig getPathTracingConfig() const;
    EIntegraotrType getIntegratorType() const;
    void getSceneLoadingConfig(SceneLoadingConfig& config) const;
    std::string getScenePath() const;
    RenderConfig(const Json & json);
    RenderConfig(const std::string & path);
    void init();
    int getWindowWidth() const;
    int getWindowHeight() const;
protected:
    DDGIConfig ddgiConfig{};
    PathTracingConfig pathTracingConfig{};
    EIntegraotrType mIntegratorType{};
    // SceneLoadingConfig sceneLoadingConfig{};
    std::string scenePath;
    int window_width = 1920;
    int window_height = 1080;
    Json json;
};
