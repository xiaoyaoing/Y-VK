#include "PathIntegrator.h"

#include "imgui.h"
#include "Common/ResourceCache.h"
#include "Scene/Compoments/Camera.h"
#include "shaders/Raytracing/PT/path_commons.h"

void PathIntegrator::render(RenderGraph& renderGraph) {
    if (camera->moving())
        pcPath.frame_num = 0;

    auto& commandBuffer = renderContext->getGraphicCommandBuffer();

    renderGraph.addRaytracingPass(
        "PT pass", [&](RenderGraph::Builder& builder, RaytracingPassSettings& settings) {
        settings.accel = &tlas;
        settings.pipelineLayout = layout.get();
        settings.rTPipelineSettings.dims = {width,height,1};
        settings.rTPipelineSettings.maxDepth = 5;
        
        auto output = renderGraph.createTexture("RT output",{width,height,TextureUsage::STORAGE | TextureUsage::TRANSFER_SRC});
        builder.writeTexture(output,TextureUsage::STORAGE);
        renderGraph.getBlackBoard().put("RT",output); }, [&](RenderPassContext& context) {
            // auto buffer = renderContext->allocateBuffer(sizeof(cameraUbo),VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
            // cameraUbo.projInverse = glm::inverse(camera->proj());
            // cameraUbo.viewInverse = glm::inverse(camera->view());
            // buffer.buffer->uploadData(&cameraUbo,sizeof(cameraUbo));
            // renderContext->bindBuffer(2,*buffer.buffer,0,sizeof(cameraUbo));
            bindRaytracingResources(commandBuffer);

            auto pushConstant = toBytes(pcPath);
            renderContext->bindPushConstants(pushConstant);
            renderContext->bindImage(1, renderGraph.getBlackBoard().getImageView("RT"));
            renderContext->traceRay(commandBuffer, {width, height, 1});

            pcPath.frame_num++;
        });
}

void PathIntegrator::initScene(Scene& scene) {
    Integrator::initScene(scene);

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
    pcPath.max_depth = 3;
    pcPath.min_depth = 0;
}

void PathIntegrator::onUpdateGUI() {
    int maxDepth = pcPath.max_depth;
    int minDepth = pcPath.min_depth;
    ImGui::SliderInt("Min Depth", &minDepth, 1, pcPath.max_depth);
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
}

PathIntegrator::PathIntegrator(Device& device_) : Integrator(device_) {
    layout = std::make_unique<PipelineLayout>(device, std::vector<std::string>{
                                                          "Raytracing/PT/raygen.rgen",
                                                          "Raytracing/PT/closesthit.rchit",
                                                          "Raytracing/PT/miss.rmiss",
                                                          "Raytracing/PT/miss_shadow.rmiss",

                                                      });
}
