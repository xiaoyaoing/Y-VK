#include "Integrator.h"

#include "Scene/Compoments/Camera.h"
#include "../Utils/RTSceneUtil.h"

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
