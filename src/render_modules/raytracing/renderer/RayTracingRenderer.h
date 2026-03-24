#pragma once

#include "Common/RTConfing.h"
#include "../Integrators/Integrator.h"
#include "../scene/RTScene.h"

#include <memory>
#include <string>

class Device;
class RenderGraph;
class Scene;
class Camera;

class RTRenderer {
public:
    virtual ~RTRenderer() = default;

    void initialize(Device& device, const RTConfing& config);
    void onSceneLoaded(Device& device, Scene& scene);
    void render(RenderGraph& renderGraph, Camera& camera);
    void updateGui();
    void configureSceneLoading(SceneLoadingConfig& sceneLoadingConfig, const RTConfing& config) const;
    std::string getHdrImageToSave() const;

    virtual const char* getName() const = 0;
    virtual EIntegraotrType getType() const = 0;

protected:
    virtual std::unique_ptr<Integrator> createIntegrator(Device& device, const RTConfing& config) const = 0;

private:
    std::unique_ptr<Integrator> integrator;
    SceneUbo sceneUbo{};
    SceneUbo lastFrameSceneUbo{};
    std::shared_ptr<RTSceneEntry> rtSceneEntry;
};

class PathTracingRTRenderer final : public RTRenderer {
public:
    const char* getName() const override { return "Path Tracing"; }
    EIntegraotrType getType() const override { return ePathTracing; }

protected:
    std::unique_ptr<Integrator> createIntegrator(Device& device, const RTConfing& config) const override;
};

class DDGIRTRenderer final : public RTRenderer {
public:
    const char* getName() const override { return "DDGI"; }
    EIntegraotrType getType() const override { return eDDGI; }

protected:
    std::unique_ptr<Integrator> createIntegrator(Device& device, const RTConfing& config) const override;
};

class RestirDIRTRenderer final : public RTRenderer {
public:
    const char* getName() const override { return "ReSTIR DI"; }
    EIntegraotrType getType() const override { return eRestirDI; }

protected:
    std::unique_ptr<Integrator> createIntegrator(Device& device, const RTConfing& config) const override;
};

