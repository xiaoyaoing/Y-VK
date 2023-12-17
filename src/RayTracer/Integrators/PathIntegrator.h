#pragma once

#include "Integrator.h"
class PathIntegrator : public  Integrator
{
public:
    void render(RenderGraph& renderGraph) override;
    virtual void initScene(Scene& scene) override;
    PathIntegrator(Device& device);
    // ~PathIntegrator();
protected:
    std::unique_ptr<PipelineLayout> layout;
    struct 
    {
        glm::mat4 viewInverse;
        glm::mat4 projInverse;
    } cameraUbo;
    
};


