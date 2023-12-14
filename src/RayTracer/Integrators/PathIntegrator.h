#pragma once

#include "Integrator.h"
class PathIntegrator : public  Integrator
{
public:
    void render(RenderGraph& renderGraph) override;
    PathIntegrator(Device& device);
protected:
    PipelineLayout * layout;
    struct 
    {
        glm::mat4 viewInverse;
        glm::mat4 projInverse;
    } cameraUbo;
    
};


