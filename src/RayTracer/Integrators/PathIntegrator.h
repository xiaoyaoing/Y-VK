#pragma once

#include "Integrator.h"
#include "Common/RTConfing.h"
#include "Raytracing/PT/path_commons.h"
#include "RenderPasses/GBufferPass.h"

class PathIntegrator : public Integrator {
public:
    void render(RenderGraph& renderGraph) override;
    void initScene(RTSceneEntry & entry) override;
    void onUpdateGUI() override;

    PathIntegrator(Device& device,PathTracingConfig config);

    // ~PathIntegrator();
protected:
    GBufferPass                     gbufferPass;
    LightingPass                    lightingPass;
    std::unique_ptr<PipelineLayout> layout;
    std::unique_ptr<PipelineLayout> tem_layout;
    struct
    {
        glm::mat4 viewInverse;
        glm::mat4 projInverse;
    } cameraUbo;
    PCPath pcPath{};
};
