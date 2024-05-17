#include "Integrator.h"

#include "Scene/Compoments/Camera.h"
#include "../Utils/RTSceneUtil.h"
#include "Common/Distrib.hpp"
#include "Common/ResourceCache.h"

#include <numeric>

Integrator::Integrator(Device& device) : renderContext(g_context), device(device) {
    // init();
    // storageImage = std::make_shared<SgImage>(device,"",VkExtent3D{width,height,1},VK_FORMAT_B8G8R8A8_UNORM,VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,VMA_MEMORY_USAGE_GPU_ONLY,VK_IMAGE_VIEW_TYPE_2D);
}

void Integrator::initScene(RTSceneEntry & sceneEntry) {
    // auto sceneEntry = RTSceneUtil::convertScene(device, scene);,s
    // //  return;
    // vertexBuffer = sceneEntry.vertexBuffer;
    // normalBuffer = sceneEntry.normalBuffer;
    // uvBuffer     = sceneEntry.uvBuffer;
    // indexBuffer  = sceneEntry.indexBuffer;
    //
    // materialsBuffer     = sceneEntry.materialsBuffer;
    // primitiveMeshBuffer = sceneEntry.primitiveMeshBuffer;
    // transformBuffers    = sceneEntry.transformBuffers;
    // rtLightBuffer       = sceneEntry.rtLightBuffer;
    //
    // blases     = &(sceneEntry.blases);
    // tlas       = &(sceneEntry.tlas);
    // lights     = &(sceneEntry.lights);
    // primitives = &(sceneEntry.primitives);
    // materials  = &(sceneEntry.materials);
    entry_ = &sceneEntry;

    mScene = sceneEntry.scene;

   // sceneUboBuffer = std::make_shared<Buffer>(device, sizeof(SceneUbo), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, &sceneUbo);

    camera        = sceneEntry.scene->getCameras()[0];
    camera->flipY = false;

    width  = renderContext->getViewPortExtent().width;
    height = renderContext->getViewPortExtent().height;
}

void Integrator::init() {
    computePrimAreaLayout = &device.getResourceCache().requestPipelineLayout(std::vector<std::string>{"Raytracing/compute_triangle_area.comp"});
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

    for (auto& light : entry_->lights) {
        auto prim_idx = light.prim_idx;

        uint32_t tri_count = entry_->primitives[prim_idx].index_count / 3;
        if (!entry_->primAreaBuffers.contains(prim_idx))
            entry_->primAreaBuffers[prim_idx] = std::make_unique<Buffer>(device, sizeof(float) * tri_count, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
        std::string bufferName = ("area_distribution" + std::to_string(prim_idx));
        graph.addComputePass(

            "",
            [&](RenderGraph::Builder& builder, ComputePassSettings& settings) {
                auto areaBuffer = graph.importBuffer("area_distribution" + std::to_string(prim_idx), entry_->primAreaBuffers[prim_idx].get());
                graph.setOutput(areaBuffer);
                builder.writeBuffer(areaBuffer, BufferUsage::STORAGE);
            },
            [this, prim_idx,&graph,tri_count](RenderPassContext& context) {
                g_context->getPipelineState().setPipelineLayout(*computePrimAreaLayout);
                PC pc{entry_->primitives[prim_idx].index_offset, entry_->primitives[prim_idx].vertex_offset, entry_->primitives[prim_idx].index_count, 0, entry_->vertexBuffer->getDeviceAddress(), entry_->indexBuffer->getDeviceAddress(), entry_->primitives[prim_idx].world_matrix};
                LOGI("model matrix {} ", glm::to_string(entry_->primitives[prim_idx].world_matrix));
                g_context->bindBuffer(0, graph.getBlackBoard().getBuffer("area_distribution" + std::to_string(prim_idx))).bindPushConstants(pc).flushAndDispatch(context.commandBuffer, ceil(float(tri_count) / 16), 1, 1);
            });
    }

    auto commandBuffer = device.createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    graph.execute(commandBuffer);
    g_context->submit(commandBuffer);

    for (const auto& pair : entry_->primAreaBuffers) {
        auto           data = pair.second->getData<float>();
        Distribution1D distribution1D(data.data(), data.size());
        entry_->primAreaBuffers[pair.first]              = (distribution1D.toGpuBuffer(device));
        entry_->primitives[pair.first].area_distribution_buffer_addr = entry_->primAreaBuffers[pair.first]->getDeviceAddress();
        entry_->primitives[pair.first].area                          = std::accumulate(data.begin(), data.end(), 0.0f);
        // for(auto & light : lights){
        //     if(light.prim_idx == pair.first){
        //         light.L /= entry_->primitives[pair.first].area;
        //     }
        // }
    }
    entry_->rtLightBuffer->uploadData(entry_->lights.data());
    entry_->primitiveMeshBuffer->uploadData(entry_->primitives.data());
    entry_->primAreaBuffersInitialized = true;
}

Integrator::~Integrator() {
}

void Integrator::bindRaytracingResources(CommandBuffer& commandBuffer)

{
    g_context->bindAcceleration(0, entry_->tlas).bindBuffer(2, *entry_->sceneUboBuffer, 0, sizeof(SceneUbo)).bindBuffer(3, *entry_->sceneDescBuffer).bindBuffer(4, *entry_->rtLightBuffer);
    uint32_t arrayElement = 0;
    for (const auto& texture : mScene->getTextures()) {
        g_context->bindImageSampler(6, texture->getImage().getVkImageView(), texture->getSampler(), 0, arrayElement++);
    }
}
