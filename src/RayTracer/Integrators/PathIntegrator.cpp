#include "PathIntegrator.h"

#include "Common/ResourceCache.h"
#include "Scene/Compoments/Camera.h"

void PathIntegrator::render(RenderGraph& renderGraph)
{
    auto & commandBuffer = renderContext->getGraphicCommandBuffer();
    
    
    renderGraph.addRaytracingPass("PT pass",[&](RenderGraph::Builder & builder,RaytracingPassSettings & settings)
    {
        settings.accel = &tlas;
        settings.pipelineLayout = layout;
        settings.rTPipelineSettings.dims = {width,height,1};
        settings.rTPipelineSettings.maxDepth = 5;
        
        auto output = renderGraph.createTexture("RT output",{width,height,TextureUsage::STORAGE | TextureUsage::TRANSFER_SRC});
        builder.writeTexture(output,TextureUsage::STORAGE);
        renderGraph.getBlackBoard().put("RT",output);
    },[&](RenderPassContext & context)
    {
        auto buffer = renderContext->allocateBuffer(sizeof(cameraUbo),VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        cameraUbo.projInverse = glm::inverse(camera->matrices.perspective);
        cameraUbo.viewInverse = glm::inverse(camera->matrices.view);
        buffer.buffer->uploadData(&cameraUbo,sizeof(cameraUbo));
        renderContext->bindBuffer(2,*buffer.buffer,0,sizeof(cameraUbo));
        renderContext->bindInput(0,renderGraph.getBlackBoard().getImageView("RT"),1,0);
        renderContext->traceRay(commandBuffer,{width,height,1});
    });
}

PathIntegrator::PathIntegrator(Device& device):Integrator(device)
{
    layout = new  PipelineLayout(device,{"Raytracing/khr_ray_tracing_basic/raygen.rgen",
                                                   "Raytracing/khr_ray_tracing_basic/closesthit.rchit",
                                                 "Raytracing/khr_ray_tracing_basic/miss.rmiss"});

    
}



