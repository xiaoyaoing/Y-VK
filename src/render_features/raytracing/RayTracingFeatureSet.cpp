#include "RayTracingFeatureSet.h"

#include "Common/Config.h"
#include "Core/Shader/GlslCompiler.h"
#include "Core/RenderContext.h"
#include "Integrators/DDGIIntegrator.h"
#include "Integrators/PathIntegrator.h"
#include "Scene/Scene.h"
#include "Scene/Compoments/Camera.h"
#include "Utils/RTSceneUtil.h"
#include "imgui.h"

void RayTracingFeatureSet::initialize(Device& device, const RTConfing& config) {
    GlslCompiler::setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_6);
    GlslCompiler::forceRecompile = true;

    integrators[to_string(ePathTracing)] = std::make_unique<PathIntegrator>(device, config.getPathTracingConfig());
    integrators[to_string(eDDGI)]        = std::make_unique<DDGIIntegrator>(device, config.getDDGIConfig());

    integratorNames.clear();
    for (auto& integrator : integrators) {
        integratorNames.push_back(integrator.first);
    }
    currentIntegrator = to_string(config.getIntegratorType());
}

void RayTracingFeatureSet::configureSceneLoading(SceneLoadingConfig& sceneLoadingConfig, const RTConfing& config) const {
    sceneLoadingConfig = {.requiredVertexAttribute = {POSITION_ATTRIBUTE_NAME, INDEX_ATTRIBUTE_NAME, NORMAL_ATTRIBUTE_NAME, TEXCOORD_ATTRIBUTE_NAME},
                          .enableMergeDrawCalls    = false,
                          .indexType               = VK_INDEX_TYPE_UINT32,
                          .bufferAddressAble       = true,
                          .bufferForAccel          = true,
                          .bufferForStorage        = true};
    config.getSceneLoadingConfig(sceneLoadingConfig);
}

void RayTracingFeatureSet::onSceneLoaded(Device& device, Scene& scene) {
    scene.addDirectionalLight(glm::vec3(0.0, -1.0, 0.3), glm::vec3(1.0f), 1.5f);
    rtSceneEntry = RTSceneUtil::convertScene(device, scene);

    for (auto& integrator : integrators) {
        integrator.second->initScene(*rtSceneEntry);
        integrator.second->init();
    }
}

void RayTracingFeatureSet::render(RenderGraph& renderGraph, Camera& camera) {
    sceneUbo.projInverse = camera.projInverse();
    sceneUbo.viewInverse = camera.viewInverse();
    sceneUbo.view        = camera.view();
    sceneUbo.proj        = camera.proj();
    sceneUbo.prev_view   = lastFrameSceneUbo.view;
    sceneUbo.prev_proj   = lastFrameSceneUbo.proj;
    sceneUbo.z_near      = camera.getNearClipPlane();
    sceneUbo.z_far       = camera.getFarClipPlane();
    rtSceneEntry->sceneUboBuffer->uploadData(&sceneUbo, sizeof(sceneUbo));

    lastFrameSceneUbo = sceneUbo;
    renderGraph.createTexture(RT_IMAGE_NAME, {integrators[currentIntegrator]->getWidth(), integrators[currentIntegrator]->getHeight(), TextureUsage::STORAGE | TextureUsage::TRANSFER_SRC | TextureUsage::SAMPLEABLE | TextureUsage::COLOR_ATTACHMENT, VK_FORMAT_R32G32B32A32_SFLOAT});
    integrators[currentIntegrator]->render(renderGraph);
    if (renderGraph.getBlackBoard().contains(RT_IMAGE_NAME)) {
        renderGraph.addImageCopyPass(renderGraph.getBlackBoard().getHandle(RT_IMAGE_NAME), renderGraph.getBlackBoard().getHandle(RENDER_VIEW_PORT_IMAGE_NAME));
    }
}

void RayTracingFeatureSet::updateGui() {
    int itemCurrent = 0;
    for (int i = 0; i < integratorNames.size(); i++) {
        if (integratorNames[i] == currentIntegrator) {
            itemCurrent = i;
            break;
        }
    }

    std::vector<const char*> integratorNamesCStr;
    for (auto& integratorName : integratorNames) {
        integratorNamesCStr.push_back(integratorName.data());
    }
    ImGui::Combo("Integrators", &itemCurrent, integratorNamesCStr.data(), integratorNames.size());
    currentIntegrator = integratorNames[itemCurrent];
    integrators[currentIntegrator]->onUpdateGUI();
}

std::string RayTracingFeatureSet::getHdrImageToSave() const {
    return RT_IMAGE_NAME;
}
