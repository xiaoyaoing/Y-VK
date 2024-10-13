#pragma once
#include "Integrator.h"
#include "RenderGraph/RenderGraph.h"

#include <variant>

class IBLRenderer{};
class RayTracerRenderer{};
class DeferredRenderer{};
class DDGIRenderer{};

struct DDGIConfig {
    float hysteresis = 0.98f;
    int rays_per_probe = 256;
    float depth_sharpness = 50.0f;
    float normal_bias = 0.1f;
    float view_bias = 0.1f;
    float backface_ratio = 0.1f;
    float probe_distance = 0.5f;
    float min_frontface_dist = 0.1f;
    float max_distance;
    glm::ivec3 probe_counts;
    glm::vec3 probe_start_position;
};

using Renderer = std::variant<IBLRenderer, RayTracerRenderer, DeferredRenderer, DDGIRenderer>;

class DDGI : public Integrator {
    void render(RenderGraph& graph) override;
    DDGI();
protected:
    // class Impl;
    // Impl* impl;
};