#pragma once

#include "Integrator.h"
#include "Raytracing/PT/path_commons.h"

class PathIntegrator : public  Integrator
{
public:
    void render(RenderGraph& renderGraph) override;
    void initScene(Scene& scene) override;
    void onUpdateGUI() override;
    
    PathIntegrator(Device& device);
    
    // ~PathIntegrator();
protected:
    std::unique_ptr<PipelineLayout> layout;
    struct 
    {
        glm::mat4 viewInverse;
        glm::mat4 projInverse;
    } cameraUbo;
    PCPath pcPath{};
};


