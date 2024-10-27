#pragma once
#include "Integrator.h"
#include "Raytracing/PT/path_commons.h"
#include "Raytracing/ddgi/ddgi_commons.h"
#include "RenderGraph/RenderGraph.h"
#include "RenderPasses/GBufferPass.h"

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
    float probe_distance = 2.f;
    float min_frontface_dist = 0.1f;
    float max_distance;
    glm::ivec3 probe_counts;
    glm::vec3 probe_start_position;
};

using Renderer = std::variant<IBLRenderer, RayTracerRenderer, DeferredRenderer, DDGIRenderer>;

class DDGIIntegrator : public Integrator {
    void render(RenderGraph& graph) override;

public:
    void init() override;
    void initScene(RTSceneEntry& entry) override;
    DDGIIntegrator(Device& device) : Integrator(device) {}
    void onUpdateGUI() override;

protected:
    DDGIConfig config;
    struct DDGIBuffers;
    DDGIBuffers * buffers{nullptr};
    DDGIUbo ubo;
    PCPath pc_ray;
    uint ping = 0;
    uint pong = 1;
    std::unique_ptr<GBufferPass> gbufferPass;
    bool debugDDGI = true;
    // class Impl;
    // Impl* impl;
};