#pragma once

#include "Common/RTConfing.h"
#include "Integrators/Integrator.h"
#include "Utils/RTSceneUtil.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Device;
class RenderGraph;
class Scene;
class Camera;

class RayTracingFeatureSet {
public:
    void initialize(Device& device, const RTConfing& config);
    void onSceneLoaded(Device& device, Scene& scene);
    void render(RenderGraph& renderGraph, Camera& camera);
    void updateGui();
    void configureSceneLoading(SceneLoadingConfig& sceneLoadingConfig, const RTConfing& config) const;
    std::string getHdrImageToSave() const;

private:
    std::unordered_map<std::string, std::unique_ptr<Integrator>> integrators;
    std::string currentIntegrator;
    std::vector<std::string> integratorNames;
    SceneUbo sceneUbo{};
    SceneUbo lastFrameSceneUbo{};
    std::shared_ptr<RTSceneEntry> rtSceneEntry;
};
