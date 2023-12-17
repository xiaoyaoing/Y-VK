#pragma once

#include "Integrator.h"
class SimpleIntegrator : public  Integrator
{
public:
    void render(RenderGraph& renderGraph) override;
    virtual void initScene(Scene& scene) override;
    SimpleIntegrator(Device& device);
protected:
    PipelineLayout * layout;
    struct 
    {
        glm::mat4 viewInverse;
        glm::mat4 projInverse;
    } cameraUbo;
    
};


