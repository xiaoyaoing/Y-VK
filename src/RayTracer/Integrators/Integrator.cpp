#include "Integrator.h"

#include "Scene/Compoments/Camera.h"
#include "../Utils/RTSceneUtil.h"
#include "Common/Distrib.hpp"

#include <numeric>

Integrator::Integrator(Device& device) : renderContext(g_context), device(device) {
    // init();
    // storageImage = std::make_unique<SgImage>(device,"",VkExtent3D{width,height,1},VK_FORMAT_B8G8R8A8_UNORM,VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,VMA_MEMORY_USAGE_GPU_ONLY,VK_IMAGE_VIEW_TYPE_2D);
}

void Integrator::initScene(Scene& scene) {
    auto sceneEntry = RTSceneUtil::convertScene(device, scene);

    //  return;
    vertexBuffer = std::move(sceneEntry->vertexBuffer);
    normalBuffer = std::move(sceneEntry->normalBuffer);
    uvBuffer     = std::move(sceneEntry->uvBuffer);
    indexBuffer  = std::move(sceneEntry->indexBuffer);

    materialsBuffer     = std::move(sceneEntry->materialsBuffer);
    primitiveMeshBuffer = std::move(sceneEntry->primitiveMeshBuffer);
    transformBuffers    = std::move(sceneEntry->transformBuffers);
    rtLightBuffer       = std::move(sceneEntry->rtLightBuffer);

    blases     = std::move(sceneEntry->blases);
    tlas       = std::move(sceneEntry->tlas);
    lights     = std::move(sceneEntry->lights);
    primitives = std::move(sceneEntry->primitives);
    materials  = std::move(sceneEntry->materials);

    mScene = &scene;

    sceneUboBuffer = std::make_unique<Buffer>(device, sizeof(SceneUbo), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, &sceneUbo);

    camera        = scene.getCameras()[0];
    camera->flipY = false;

    width  = renderContext->getViewPortExtent().width;
    height = renderContext->getViewPortExtent().height;
}

void Integrator::init(Scene& scene_) {

    initScene(scene_);
    computePrimAreaLayout = std::make_unique<PipelineLayout>(device, std::vector<std::string>{"Raytracing/compute_triangle_area.comp"});
}

void Integrator::updateGui() {
}

void Integrator::destroy() {
}

void Integrator::update() {
}
void Integrator::initLightAreaDistribution(RenderGraph& _graph) {
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
                LOGI("model matrix {} ", glm::to_string(primitives[prim_idx].world_matrix));
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

Integrator::~Integrator() {
}

void Integrator::bindRaytracingResources(CommandBuffer& commandBuffer)

{
    sceneUbo.projInverse = camera->projInverse();
    sceneUbo.viewInverse = camera->viewInverse();
    sceneUboBuffer->uploadData(&sceneUbo, sizeof(sceneUbo));

    g_context->bindAcceleration(0, tlas).bindBuffer(2, *sceneUboBuffer, 0, sizeof(sceneUbo)).bindBuffer(3, *sceneDescBuffer).bindBuffer(4, *rtLightBuffer);
    uint32_t arrayElement = 0;
    for (const auto& texture : mScene->getTextures()) {
        g_context->bindImageSampler(6, texture->getImage().getVkImageView(), texture->getSampler(), 0, arrayElement++);
    }
}
