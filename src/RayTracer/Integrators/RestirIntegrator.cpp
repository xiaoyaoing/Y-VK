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
    if (!primAreaBuffersInitialized) {
        initLightAreaDistribution(renderGraph);
    }

    if (camera->moving())
        pcPath.frame_num = 0;

    auto& commandBuffer = renderContext->getGraphicCommandBuffer();

    renderGraph.addRaytracingPass(
        "Restir temporal pass", [&](RenderGraph::Builder& builder, RaytracingPassSettings& settings) {
        settings.accel = &tlas;
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

    return;
    renderGraph.addRaytracingPass(
       "Restir spatial pass", [&](RenderGraph::Builder& builder, RaytracingPassSettings& settings) {
       settings.accel = &tlas;
       settings.pipelineLayout = spatialLayout.get();
       settings.rTPipelineSettings.dims = {width,height,1};
       settings.rTPipelineSettings.maxDepth = 5;
    
    
        }, [&](RenderPassContext& context) {
           bindRaytracingResources(commandBuffer);
           renderContext->bindPushConstants(pcPath);
           renderContext->bindImage(0, renderGraph.getBlackBoard().getImageView("RT"));
        renderContext->traceRay(commandBuffer, {width, height, 1});
    
           pcPath.frame_num++; });


    renderGraph.addRaytracingPass(
       "Restir output pass", [&](RenderGraph::Builder& builder, RaytracingPassSettings& settings) {
       settings.accel = &tlas;
       settings.pipelineLayout = outputLayout.get();
       settings.rTPipelineSettings.dims = {width,height,1};
       settings.rTPipelineSettings.maxDepth = 5;
    }, [&](RenderPassContext& context) {
           bindRaytracingResources(commandBuffer);
           renderContext->bindPushConstants(pcPath);
           renderContext->bindImage(0, renderGraph.getBlackBoard().getImageView("RT"));
        renderContext->traceRay(commandBuffer, {width, height, 1});
    
           pcPath.frame_num++; });
}
void RestirIntegrator::initScene(Scene& scene) {
    Integrator::initScene(scene);

    temporReservoirBuffer  = std::make_unique<Buffer>(device, sizeof(RestirReservoir) * width * height, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    spatialReservoirBuffer = std::make_unique<Buffer>(device, sizeof(RestirReservoir) * width * height, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    passReservoirBuffer    = std::make_unique<Buffer>(device, sizeof(RestirReservoir) * width * height, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

    SceneDesc desc{
        .vertex_addr                    = vertexBuffer->getDeviceAddress(),
        .index_addr                     = indexBuffer->getDeviceAddress(),
        .normal_addr                    = normalBuffer->getDeviceAddress(),
        .uv_addr                        = uvBuffer->getDeviceAddress(),
        .material_addr                  = materialsBuffer->getDeviceAddress(),
        .prim_info_addr                 = primitiveMeshBuffer->getDeviceAddress(),
        .restir_temporal_reservoir_addr = temporReservoirBuffer->getDeviceAddress(),
        .restir_spatial_reservoir_addr  = spatialReservoirBuffer->getDeviceAddress(),
        .restir_pass_reservoir_addr     = passReservoirBuffer->getDeviceAddress(),
    };
    sceneDescBuffer = std::make_unique<Buffer>(device, sizeof(SceneDesc), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, &desc);

    pcPath.light_num = lights.size();
    pcPath.max_depth = 1;
    pcPath.min_depth = 0;
    pcPath.do_spatial_reuse = false;
    pcPath.do_temporal_reuse = false;
}
void RestirIntegrator::onUpdateGUI() {
    Integrator::onUpdateGUI();
    ImGui::Checkbox("Do temporal reuse", reinterpret_cast<bool*>(&pcPath.do_temporal_reuse));
    ImGui::Checkbox("Do spatial reuse", reinterpret_cast<bool*>(&pcPath.do_spatial_reuse));
}
