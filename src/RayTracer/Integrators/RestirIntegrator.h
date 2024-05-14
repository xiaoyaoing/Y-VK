#pragma once
#include "Integrator.h"
#include "Raytracing/RestirDI/di_commons.h"
class RestirIntegrator : public Integrator {
public:
    RestirIntegrator(Device& device);
    void render(RenderGraph& renderGraph) override;
    void initScene(RTSceneEntry & entry) override;
    void onUpdateGUI() override;
protected:
    RestirDIPC pcPath{};
    std::unique_ptr<PipelineLayout> temporalLayout;
    std::unique_ptr<PipelineLayout> spatialLayout;
    std::unique_ptr<PipelineLayout> outputLayout;

    std::unique_ptr<Buffer> temporReservoirBuffer{nullptr};
    std::unique_ptr<Buffer> spatialReservoirBuffer{nullptr};
    std::unique_ptr<Buffer> passReservoirBuffer{nullptr};
};
