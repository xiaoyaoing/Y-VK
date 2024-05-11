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
    if (!primAreaBuffersInitialized) {
        initLightAreaDistribution(renderGraph);
    }

    if (camera->moving())
        pcPath.frame_num = 0;

    auto& commandBuffer = renderContext->getGraphicCommandBuffer();

    struct PC {
        uint32_t index_offset;
        uint32_t index_count;
        uint64_t vertex_address;
        uint64_t index_address;
        mat4     model;
    };

    // uint32_t tri_count        = primitives[prim_idx].index_count / 3;
    // primAreaBuffers[prim_idx] = std::make_unique<Buffer>(device, sizeof(float) * tri_count, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    // std::string bufferName    = ("area_distribution" + std::to_string(prim_idx));
    // for (int i = 0; i < 10; i++)
    //     graph.addComputePass(
    //         "",
    //         [&](RenderGraph::Builder& builder, ComputePassSettings& settings) {
    //             LOGI("Adding compute pass");
    //             auto areaBuffer = graph.importBuffer(("area_distribution" + std::to_string(i)).c_str(), primAreaBuffers[prim_idx].get());
    //             graph.setOutput(areaBuffer);
    //             builder.writeBuffer(areaBuffer, BufferUsage::STORAGE);
    //         },
    //         [&, prim_idx, i](RenderPassContext& context) {
    //             g_context->getPipelineState().setPipelineLayout(*pipelineLayout);
    //             PC pc{primitives[prim_idx].index_offset, primitives[prim_idx].index_count, vertexBuffer->getDeviceAddress(), indexBuffer->getDeviceAddress(), primitives[prim_idx].world_matrix};
    //             LOGI("Dispatching compute pass")
    //             g_context->bindBuffer(0, graph.getBlackBoard().getBuffer("area_distribution" + std::to_string(i))).bindPushConstants(pc).flushAndDispatch(context.commandBuffer, ceil(float(tri_count) / 16), 1, 1);
    //         });
    //
    // RenderGraph graph(device);
    // auto        cmd = device.createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    //
    // auto     prim_idx    = 0;
    // uint32_t tri_count   = primitives[prim_idx].index_count / 3;
    // auto     temp_buffer = new Buffer(device, sizeof(float) * tri_count, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    // graph.addComputePass(
    //     "",
    //     [&](RenderGraph::Builder& builder, ComputePassSettings& settings) {
    //         auto areaBuffer = graph.importBuffer("area_distribution", temp_buffer);
    //         graph.setOutput(areaBuffer);
    //         builder.writeBuffer(areaBuffer, BufferUsage::STORAGE);
    //     },
    //     [&, prim_idx](RenderPassContext& context) {
    //         g_context->getPipelineState().setPipelineLayout(*tem_layout);
    //         PC pc{primitives[prim_idx].index_offset, primitives[prim_idx].index_count, vertexBuffer->getDeviceAddress(), indexBuffer->getDeviceAddress(), primitives[prim_idx].world_matrix};
    //         g_context->bindBuffer(0, graph.getBlackBoard().getBuffer("area_distribution")).bindPushConstants(pc).flushAndDispatch(context.commandBuffer, ceil(float(tri_count) / 16), 1, 1);
    //     });
    // graph.execute(cmd);
    //  g_context->submit(cmd);

    // gbufferPass.render(renderGraph);
    // lightingPass.render(renderGraph);

    renderGraph.addRaytracingPass(
        "PT pass", [&](RenderGraph::Builder& builder, RaytracingPassSettings& settings) {
        settings.accel = &tlas;
        settings.pipelineLayout = layout.get();
        settings.rTPipelineSettings.dims = {width,height,1};
        settings.rTPipelineSettings.maxDepth = 5;
    
    
        auto output = renderGraph.createTexture("RT output",{width,height,TextureUsage::STORAGE | TextureUsage::TRANSFER_SRC,VK_FORMAT_R32G32B32A32_SFLOAT});
        builder.writeTexture(output,TextureUsage::STORAGE);
        renderGraph.getBlackBoard().put("RT",output); }, [&](RenderPassContext& context) {
            // auto buffer = renderContext->allocateBuffer(sizeof(cameraUbo),VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
            // cameraUbo.projInverse = glm::inverse(camera->matrices.perspective);
            // cameraUbo.viewInverse = glm::inverse(camera->matrices.view);F
            // buffer.buffer->uploadData(&cameraUbo,sizeof(cameraUbo));
            // renderContext->bindBuffer(2,*buffer.buffer,0,sizeof(cameraUbo));
            bindRaytracingResources(commandBuffer);
    
            auto pushConstant = toBytes(pcPath);
            renderContext->bindPushConstants(pushConstant);
            renderContext->bindImage(0, renderGraph.getBlackBoard().getImageView("RT"));
         renderContext->traceRay(commandBuffer, {width, height, 1});
    
            pcPath.frame_num++; });
}
void PathIntegrator::initLightAreaDistribution(RenderGraph& graph_) {

    RenderGraph graph(device);
    // RenderGraph graph(device);
    struct PC {
        uint32_t index_offset;
        uint32_t vertex_offset;
        uint32_t index_count;
        uint32_t padding;
        uint64_t vertex_address;
        uint64_t index_address;
        mat4     model;
    };

    for (auto& light : lights) {
        auto prim_idx = light.prim_idx;

        uint32_t tri_count = primitives[prim_idx].index_count / 3;
        if (!primAreaBuffers.contains(prim_idx))
            primAreaBuffers[prim_idx] = std::make_unique<Buffer>(device, sizeof(float) * tri_count, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
        std::string bufferName = ("area_distribution" + std::to_string(prim_idx));
        graph.addComputePass(

            "",
            [&](RenderGraph::Builder& builder, ComputePassSettings& settings) {
                auto areaBuffer = graph.importBuffer("area_distribution" + std::to_string(prim_idx), primAreaBuffers[prim_idx].get());
                graph.setOutput(areaBuffer);
                builder.writeBuffer(areaBuffer, BufferUsage::STORAGE);
            },
            [this, prim_idx,&graph,tri_count](RenderPassContext& context) {
                g_context->getPipelineState().setPipelineLayout(*computePrimAreaLayout);
                PC pc{primitives[prim_idx].index_offset, primitives[prim_idx].vertex_offset, primitives[prim_idx].index_count, 0, vertexBuffer->getDeviceAddress(), indexBuffer->getDeviceAddress(), primitives[prim_idx].world_matrix};
                g_context->bindBuffer(0, graph.getBlackBoard().getBuffer("area_distribution" + std::to_string(prim_idx))).bindPushConstants(pc).flushAndDispatch(context.commandBuffer, ceil(float(tri_count) / 16), 1, 1);
            });
    }

    auto commandBuffer = device.createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    graph.execute(commandBuffer);
    g_context->submit(commandBuffer);

    for (const auto& pair : primAreaBuffers) {
        auto           data = pair.second->getData<float>();
        Distribution1D distribution1D(data.data(), data.size());
        primAreaDistributionBuffers[pair.first]              = (distribution1D.toGpuBuffer(device));
        primitives[pair.first].area_distribution_buffer_addr = primAreaDistributionBuffers[pair.first]->getDeviceAddress();
        primitives[pair.first].area                          = std::accumulate(data.begin(), data.end(), 0.0f);
        // for(auto & light : lights){
        //     if(light.prim_idx == pair.first){
        //         light.L /= primitives[pair.first].area;
        //     }
        // }
    }
    rtLightBuffer->uploadData(lights.data());
    primitiveMeshBuffer->uploadData(primitives.data());
    primAreaBuffersInitialized = true;
}

void PathIntegrator::initScene(Scene& scene) {
    Integrator::initScene(scene);

    //  return;

    SceneDesc desc{
        .vertex_addr    = vertexBuffer->getDeviceAddress(),
        .index_addr     = indexBuffer->getDeviceAddress(),
        .normal_addr    = normalBuffer->getDeviceAddress(),
        .uv_addr        = uvBuffer->getDeviceAddress(),
        .material_addr  = materialsBuffer->getDeviceAddress(),
        .prim_info_addr = primitiveMeshBuffer->getDeviceAddress(),
    };
    sceneDescBuffer = std::make_unique<Buffer>(device, sizeof(SceneDesc), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, &desc);

    pcPath.light_num = lights.size();
    pcPath.max_depth = 1;
    pcPath.min_depth = 0;

    gbufferPass.init();
    lightingPass.init();

    /// pcPath.max_depth = 5;
    //  pcPath.min_depth = 0;
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
}

PathIntegrator::PathIntegrator(Device& device_) : Integrator(device_) {
    layout = std::make_unique<PipelineLayout>(device, std::vector<std::string>{
                                                          "Raytracing/PT/raygen.rgen",
                                                          "Raytracing/PT/miss.rmiss",
                                                          "Raytracing/PT/miss_shadow.rmiss",
                                                          "Raytracing/PT/closesthit.rchit",
                                                          "Raytracing/ray.rahit",

                                                      });

    std::vector<Shader> shaders = {Shader(device, FileUtils::getShaderPath("Raytracing/compute_triangle_area.comp"))};
    tem_layout                  = std::make_unique<PipelineLayout>(device, shaders);

    pcPath.enable_sample_bsdf  = 0;
    pcPath.enable_sample_light = 1;
}
