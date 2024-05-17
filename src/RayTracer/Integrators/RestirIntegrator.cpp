#include "RestirIntegrator.h"

#include "Common/Distrib.hpp"
#include "Common/ResourceCache.h"
#include "Scene/Compoments/Camera.h"
RestirIntegrator::RestirIntegrator(Device& device) : Integrator(device) {
    temporalLayout = std::make_unique<PipelineLayout>(device, std::vector<std::string>{
                                                                  "Raytracing/RestirDI/tempor_reuse.rgen",
                                                                  "Raytracing/PT/miss.rmiss",
                                                                  "Raytracing/PT/miss_shadow.rmiss",
                                                                  "Raytracing/PT/closesthit.rchit",
                                                                  "Raytracing/ray.rahit",

                                                              });

    spatialLayout = std::make_unique<PipelineLayout>(device, std::vector<std::string>{
                                                                 "Raytracing/RestirDI/spatial_reuse.rgen",
                                                                 "Raytracing/PT/miss.rmiss",
                                                                 "Raytracing/PT/miss_shadow.rmiss",
                                                                 "Raytracing/PT/closesthit.rchit",
                                                                 "Raytracing/ray.rahit",

                                                             });

    outputLayout = std::make_unique<PipelineLayout>(device, std::vector<std::string>{
                                                                "Raytracing/RestirDI/output.rgen",
                                                                "Raytracing/PT/miss.rmiss",
                                                                "Raytracing/PT/miss_shadow.rmiss",
                                                                "Raytracing/PT/closesthit.rchit",
                                                                "Raytracing/ray.rahit",

                                                            });


 
}

void RestirIntegrator::render(RenderGraph& renderGraph) {
    if (!entry_->primAreaBuffersInitialized) {
        initLightAreaDistribution(renderGraph);
    }

    if (camera->moving())
        pcPath.frame_num = 0;

    auto& commandBuffer = renderContext->getGraphicCommandBuffer();




    renderGraph.addRaytracingPass(
        "Restir temporal pass", [&](RenderGraph::Builder& builder, RaytracingPassSettings& settings) {
        settings.accel = &entry_->tlas;
        settings.pipelineLayout = temporalLayout.get();
        settings.rTPipelineSettings.dims = {width,height,1};
        settings.rTPipelineSettings.maxDepth = 5;
    
    
        auto output = renderGraph.createTexture("RT output",{width,height,TextureUsage::STORAGE | TextureUsage::TRANSFER_SRC,VK_FORMAT_R32G32B32A32_SFLOAT});
        builder.writeTexture(output,TextureUsage::STORAGE);
        renderGraph.getBlackBoard().put("RT",output); }, [&](RenderPassContext& context) {
            bindRaytracingResources(commandBuffer);
            renderContext->bindPushConstants(pcPath);
            renderContext->bindImage(0, renderGraph.getBlackBoard().getImageView("RT"));
         renderContext->traceRay(commandBuffer, {width, height, 1});
    
            pcPath.frame_num++; });

    renderGraph.addRaytracingPass(
       "Restir spatial pass", [&](RenderGraph::Builder& builder, RaytracingPassSettings& settings) {
        settings.accel = &entry_->tlas;
       settings.pipelineLayout = spatialLayout.get();
       settings.rTPipelineSettings.dims = {width,height,1};
       settings.rTPipelineSettings.maxDepth = 5;
    
    
        }, [&](RenderPassContext& context) {
           bindRaytracingResources(commandBuffer);
           renderContext->bindPushConstants(pcPath);
           renderContext->bindImage(0, renderGraph.getBlackBoard().getImageView("RT"));
        renderContext->traceRay(commandBuffer, {width, height, 1});
    
      });


    renderGraph.addRaytracingPass(
       "Restir output pass", [&](RenderGraph::Builder& builder, RaytracingPassSettings& settings) {
        settings.accel = &entry_->tlas;
       settings.pipelineLayout = outputLayout.get();
       settings.rTPipelineSettings.dims = {width,height,1};
       settings.rTPipelineSettings.maxDepth = 5;
    }, [&](RenderPassContext& context) {
           bindRaytracingResources(commandBuffer);
           renderContext->bindPushConstants(pcPath);
           renderContext->bindImage(0, renderGraph.getBlackBoard().getImageView("RT"));
        renderContext->traceRay(commandBuffer, {width, height, 1});
    
           });
}
void RestirIntegrator::initScene(RTSceneEntry & entry) {
    Integrator::initScene(entry);

    temporReservoirBuffer  = std::make_unique<Buffer>(device, sizeof(RestirReservoir) * width * height, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    spatialReservoirBuffer = std::make_unique<Buffer>(device, sizeof(RestirReservoir) * width * height, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    passReservoirBuffer    = std::make_unique<Buffer>(device, sizeof(RestirReservoir) * width * height, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    gBuffer                = std::make_unique<Buffer>(device, sizeof(GBuffer) * width * height, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY);


    entry.sceneDesc.restir_temporal_reservoir_addr = temporReservoirBuffer->getDeviceAddress();
    entry.sceneDesc.restir_spatial_reservoir_addr  = spatialReservoirBuffer->getDeviceAddress();
    entry.sceneDesc.restir_pass_reservoir_addr     = passReservoirBuffer->getDeviceAddress();
    entry.sceneDesc.gbuffer_addr                  = gBuffer->getDeviceAddress();
    entry.sceneDescBuffer->uploadData(&entry.sceneDesc, sizeof(SceneDesc));
    
    pcPath.light_num = entry.lights.size();
    pcPath.max_depth = 1;
    pcPath.min_depth = 0;
    pcPath.frame_num = 0;
    pcPath.do_spatial_reuse = false;
    pcPath.do_temporal_reuse = true;
}
void RestirIntegrator::onUpdateGUI() {
    Integrator::onUpdateGUI();
    ImGui::Checkbox("Do temporal reuse", reinterpret_cast<bool*>(&pcPath.do_temporal_reuse));
    ImGui::Checkbox("Do spatial reuse", reinterpret_cast<bool*>(&pcPath.do_spatial_reuse));
}
