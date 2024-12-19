//
// Created by pc on 2023/12/1.
//
#include "Scene/RuntimeSceneManager.h"

#include "RayTracer.h"

#include "imgui.h"
#include "Common/Config.h"
#include "Common/ResourceCache.h"
#include "Common/VkCommon.h"
#include "Core/Shader/GlslCompiler.h"
#include "Integrators/DDGIIntegrator.h"
#include "Integrators/PathIntegrator.h"
#include "Integrators/RestirIntegrator.h"
#include "Integrators/SimpleIntegrator.h"
#include "Scene/SceneLoader/SceneLoaderInterface.h"
#include "Scene/SceneLoader/gltfloader.h"

RayTracer::RayTracer(const RTConfing& settings) : Application("Real time Ray tracer", settings.getWindowWidth(), settings.getWindowHeight(),settings) {
    addDeviceExtension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    addDeviceExtension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    addDeviceExtension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
    addDeviceExtension(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
    addDeviceExtension(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);
    addDeviceExtension(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
    addDeviceExtension(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    addDeviceExtension(VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME);
    addDeviceExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);

    //Application::g_App = this;
}

void RayTracer::drawFrame(RenderGraph& renderGraph) {

    sceneUbo.projInverse = camera->projInverse();
    sceneUbo.viewInverse = camera->viewInverse();
    sceneUbo.view        = camera->view();
    sceneUbo.proj        = camera->proj();
    sceneUbo.prev_view   = lastFrameSceneUbo.view;
    sceneUbo.prev_proj   = lastFrameSceneUbo.proj;
    sceneUbo.z_near      = camera->getNearClipPlane();
    sceneUbo.z_far       = camera->getFarClipPlane();
    rtSceneEntry->sceneUboBuffer->uploadData(&sceneUbo, sizeof(sceneUbo));

    lastFrameSceneUbo = sceneUbo;
    renderGraph.createTexture(RT_IMAGE_NAME, {integrators[currentIntegrator]->width, integrators[currentIntegrator]->height, TextureUsage::STORAGE | TextureUsage::TRANSFER_SRC | TextureUsage::SAMPLEABLE | TextureUsage::COLOR_ATTACHMENT, VK_FORMAT_R32G32B32A32_SFLOAT});
    integrators[currentIntegrator]->render(renderGraph);
    if (renderGraph.getBlackBoard().contains(RT_IMAGE_NAME))
        renderGraph.addImageCopyPass(renderGraph.getBlackBoard().getHandle(RT_IMAGE_NAME), renderGraph.getBlackBoard().getHandle(RENDER_VIEW_PORT_IMAGE_NAME));
}

void RayTracer::onSceneLoaded() {
    // scene->addDirectionalLight(glm::vec3(0.0, -1.0, 0.3), glm::vec3(1.0f), 1.5f);
    scene->addDirectionalLight(glm::vec3(0.6,-0.435,-0.816), glm::vec3(1.0f), 1.0f);
    camera = scene->getCameras()[0];
    Config::GetInstance().CameraFromConfig(*camera, scene->getName());
    sceneFirstLoad = false;

    rtSceneEntry = RTSceneUtil::convertScene(*device, *scene);
    for (auto& integrator : integrators) {
        integrator.second->initScene(*rtSceneEntry);
        integrator.second->init();
    }

    initView();
}
std::string RayTracer::getHdrImageToSave() {
    return RT_IMAGE_NAME;
}

void RayTracer::prepare() {
    Application::prepare();
    GlslCompiler::setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_6);
    GlslCompiler::forceRecompile = true;

    integrators[to_string(ePathTracing)] = std::make_unique<PathIntegrator>(*device, config.getPathTracingConfig());
    integrators[to_string(eDDGI)]        = std::make_unique<DDGIIntegrator>(*device, config.getDDGIConfig());

    for (auto& integrator : integrators) {
        integratorNames.push_back(integrator.first);
    }

    currentIntegrator = to_string(config.getIntegratorType());

    sceneLoadingConfig = {.requiredVertexAttribute = {POSITION_ATTRIBUTE_NAME, INDEX_ATTRIBUTE_NAME, NORMAL_ATTRIBUTE_NAME, TEXCOORD_ATTRIBUTE_NAME},
                          .enableMergeDrawCalls    = false,
                          .indexType               = VK_INDEX_TYPE_UINT32,
                          .bufferAddressAble       = true,
                          .bufferForAccel          = true,
                          .bufferForStorage        = true,.loadLight =  false};
    config.getSceneLoadingConfig(sceneLoadingConfig);
    loadScene(config.getScenePath());
    g_context->setFlipViewport(false);
}
void RayTracer::onUpdateGUI() {
    Application::onUpdateGUI();

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

int main() {
    Json config = JsonUtil::fromFile(FileUtils::getResourcePath("render.json"));
    
    RayTracer rayTracer(config);
    rayTracer.prepare();
    rayTracer.mainloop();
    return 0;
}