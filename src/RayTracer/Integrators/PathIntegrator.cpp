#include "PathIntegrator.h"

#include "imgui.h"
#include "Common/Distrib.hpp"
#include "Common/ResourceCache.h"
#include "Scene/Compoments/Camera.h"
#include "Raytracing/PT/path_commons.h"
#include "RenderPasses/GBufferPass.h"

#include <numeric>

#define OFFSET(structure, member) ((int64_t) & ((structure*)0)->member)// 64位系统

void PathIntegrator::render(RenderGraph& renderGraph) {
    if (!entry_->primAreaBuffersInitialized) {
        initLightAreaDistribution(renderGraph);
    }

    if (camera->moving())
        pcPath.frame_num = 0;

    auto& commandBuffer = renderContext->getGraphicCommandBuffer();

    renderGraph.addRaytracingPass(
        "PT pass", [&](RenderGraph::Builder& builder, RaytracingPassSettings& settings) {
        settings.accel = &entry_->tlas;
        settings.pipelineLayout = layout.get();
        settings.rTPipelineSettings.dims = {width,height,1};
        settings.rTPipelineSettings.maxDepth = 5;
    
    
        auto output = renderGraph.createTexture(RT_IMAGE_NAME,{width,height,TextureUsage::STORAGE | TextureUsage::TRANSFER_SRC | TextureUsage::SAMPLEABLE,VK_FORMAT_R32G32B32A32_SFLOAT});
        builder.writeTexture(output,TextureUsage::STORAGE); }, [&](RenderPassContext& context) {
            bindRaytracingResources(commandBuffer);
    
            auto pushConstant = toBytes(pcPath);
            renderContext->bindPushConstants(pushConstant);
            renderContext->bindImage(0, renderGraph.getBlackBoard().getImageView(RT_IMAGE_NAME));
            renderContext->traceRay(commandBuffer, {width, height, 1});
    
            pcPath.frame_num++; });
}

void PathIntegrator::initScene(RTSceneEntry& entry) {
    Integrator::initScene(entry);

    //  return;

    // SceneDesc desc{
    //     .vertex_addr    = vertexBuffer->getDeviceAddress(),
    //     .index_addr     = indexBuffer->getDeviceAddress(),
    //     .normal_addr    = normalBuffer->getDeviceAddress(),
    //     .uv_addr        = uvBuffer->getDeviceAddress(),
    //     .material_addr  = materialsBuffer->getDeviceAddress(),
    //     .prim_info_addr = primitiveMeshBuffer->getDeviceAddress(),
    // };
    // sceneDescBuffer = std::make_unique<Buffer>(device, sizeof(SceneDesc), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, &desc);

    pcPath.light_num = entry_->lights.size();
    pcPath.max_depth = 5;
    pcPath.min_depth = 0;
}

void PathIntegrator::onUpdateGUI() {
    int maxDepth = pcPath.max_depth;
    int minDepth = pcPath.min_depth;
    ImGui::SliderInt("Min Depth", &minDepth, 0, pcPath.max_depth);
    ImGui::SliderInt("Max Depth", &maxDepth, 1, 10);
    if (maxDepth != pcPath.max_depth) {
        pcPath.max_depth = maxDepth;
        pcPath.frame_num = 0;
    }
    if (minDepth != pcPath.min_depth) {
        pcPath.min_depth = minDepth;
        pcPath.frame_num = 0;
    }
    ImGui::Text("frame count %5d", pcPath.frame_num);
    ImGui::Checkbox("Sample BSDF", reinterpret_cast<bool*>(&pcPath.enable_sample_bsdf));
    ImGui::SameLine();
    ImGui::Checkbox("Sample Light", reinterpret_cast<bool*>(&pcPath.enable_sample_light));
    ImGui::Checkbox("Enable Accumulation", reinterpret_cast<bool*>(&pcPath.enable_accumulation));
    ImGui::Checkbox("Visual Throughput", reinterpret_cast<bool*>(&pcPath.visual_throughput));
    ImGui::Checkbox("Visual Normal", reinterpret_cast<bool*>(&pcPath.visual_normal));
    ImGui::Checkbox("Visual Material Type", reinterpret_cast<bool*>(&pcPath.visual_material_type));
    ImGui::Checkbox("Visual Albedo", reinterpret_cast<bool*>(&pcPath.visual_albedo));
}

PathIntegrator::PathIntegrator(Device& device_) : Integrator(device_) {
    layout = std::make_unique<PipelineLayout>(device, ShaderPipelineKey{
                                                          "Raytracing/PT/raygen.rgen",
                                                          "Raytracing/PT/miss.rmiss",
                                                          "Raytracing/PT/miss_shadow.rmiss",
                                                          "Raytracing/PT/closesthit.rchit",
                                                          "Raytracing/ray.rahit",
                                                      });

    tem_layout = std::make_unique<PipelineLayout>(device, ShaderPipelineKey({"Raytracing/compute_triangle_area.comp"}));

    pcPath.enable_sample_bsdf  = 1;
    pcPath.enable_sample_light = 1;
    pcPath.enable_accumulation = true;
}
