#include "RayTracingRenderer.h"

#include "Common/RTConfing.h"
#include "Core/RenderContext.h"
#include "Core/Shader/GlslCompiler.h"
#include "Scene/Compoments/Camera.h"
#include "Scene/Scene.h"
#include "imgui.h"
#include "../Integrators/DDGIIntegrator.h"
#include "../Integrators/PathIntegrator.h"
#include "../Integrators/RestirIntegrator.h"
#include "../scene/RTSceneBuilder.h"

void RTRenderer::initialize(Device& device, const RTConfing& config) {
    GlslCompiler::setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_6);
    GlslCompiler::forceRecompile = true;
    integrator = createIntegrator(device, config);
}

void RTRenderer::configureSceneLoading(SceneLoadingConfig& sceneLoadingConfig, const RTConfing& config) const {
    sceneLoadingConfig = {.requiredVertexAttribute = {POSITION_ATTRIBUTE_NAME, INDEX_ATTRIBUTE_NAME, NORMAL_ATTRIBUTE_NAME, TEXCOORD_ATTRIBUTE_NAME},
                          .enableMergeDrawCalls = false,
                          .indexType = VK_INDEX_TYPE_UINT32,
                          .bufferAddressAble = true,
                          .bufferForAccel = true,
                          .bufferForStorage = true};
    config.getSceneLoadingConfig(sceneLoadingConfig);
}

void RTRenderer::onSceneLoaded(Device& device, Scene& scene) {
    rtSceneEntry = RTSceneBuilder::build(device, scene);
    integrator->initScene(*rtSceneEntry);
    integrator->init();
}

void RTRenderer::render(RenderGraph& renderGraph, Camera& camera) {
    sceneUbo.projInverse = camera.projInverse();
    sceneUbo.viewInverse = camera.viewInverse();
    sceneUbo.view = camera.view();
    sceneUbo.proj = camera.proj();
    sceneUbo.prev_view = lastFrameSceneUbo.view;
    sceneUbo.prev_proj = lastFrameSceneUbo.proj;
    sceneUbo.z_near = camera.getNearClipPlane();
    sceneUbo.z_far = camera.getFarClipPlane();
    rtSceneEntry->sceneUboBuffer->uploadData(&sceneUbo, sizeof(sceneUbo));

    lastFrameSceneUbo = sceneUbo;
    renderGraph.createTexture(RT_IMAGE_NAME, {integrator->getWidth(), integrator->getHeight(), TextureUsage::STORAGE | TextureUsage::TRANSFER_SRC | TextureUsage::SAMPLEABLE | TextureUsage::COLOR_ATTACHMENT, VK_FORMAT_R32G32B32A32_SFLOAT});
    integrator->render(renderGraph);
    if (renderGraph.getBlackBoard().contains(RT_IMAGE_NAME)) {
        renderGraph.addImageCopyPass(renderGraph.getBlackBoard().getHandle(RT_IMAGE_NAME), renderGraph.getBlackBoard().getHandle(RENDER_VIEW_PORT_IMAGE_NAME));
    }
}

void RTRenderer::updateGui() {
    integrator->onUpdateGUI();
}

std::string RTRenderer::getHdrImageToSave() const {
    return RT_IMAGE_NAME;
}

std::unique_ptr<Integrator> PathTracingRTRenderer::createIntegrator(Device& device, const RTConfing& config) const {
    return std::make_unique<PathIntegrator>(device, config.getPathTracingConfig());
}

std::unique_ptr<Integrator> DDGIRTRenderer::createIntegrator(Device& device, const RTConfing& config) const {
    return std::make_unique<DDGIIntegrator>(device, config.getDDGIConfig());
}

std::unique_ptr<Integrator> RestirDIRTRenderer::createIntegrator(Device& device, const RTConfing&) const {
    return std::make_unique<RestirIntegrator>(device);
}

