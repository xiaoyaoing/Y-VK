//
// Created by pc on 2023/12/1.
//
#include "Scene/RuntimeSceneManager.h"

#include "RayTracer.h"

#include "Common/Config.h"
#include "Common/ResourceCache.h"
#include "Common/VkCommon.h"
#include "Core/Shader/GlslCompiler.h"
#include "Integrators/PathIntegrator.h"
#include "Integrators/RestirIntegrator.h"
#include "Integrators/SimpleIntegrator.h"
#include "PostProcess/PostProcess.h"
#include "Scene/SceneLoader/SceneLoaderInterface.h"
#include "Scene/SceneLoader/gltfloader.h"

RayTracer::RayTracer(const RayTracerSettings& settings) : Application("Real time Ray tracer", 1920, 1080) {
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
    rtSceneEntry->sceneUboBuffer->uploadData(&sceneUbo, sizeof(sceneUbo));

    lastFrameSceneUbo = sceneUbo;
    integrators[currentIntegrator]->render(renderGraph);
    // postProcess->render(renderGraph);

    renderGraph.addImageCopyPass(renderGraph.getBlackBoard().getHandle(RT_IMAGE_NAME), renderGraph.getBlackBoard().getHandle(RENDER_VIEW_PORT_IMAGE_NAME));
}
void RayTracer::onSceneLoaded() {
    camera = scene->getCameras()[0];
    Config::GetInstance().CameraFromConfig(*camera, scene->getName());
    sceneFirstLoad = false;

    rtSceneEntry = RTSceneUtil::convertScene(*device, *scene);
    for (auto& integrator : integrators) {
        integrator.second->initScene(*rtSceneEntry);
        integrator.second->init();
    }
}

void RayTracer::prepare() {
    Application::prepare();
    GlslCompiler::setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_5);
    GlslCompiler::forceRecompile = true;

    integrators["path"]   = std::make_unique<PathIntegrator>(*device);
    integrators["restir"] = std::make_unique<RestirIntegrator>(*device);
    integratorNames       = {"path", "restir"};
    currentIntegrator     = "resitr";

    sceneLoadingConfig = {.requiredVertexAttribute = {POSITION_ATTRIBUTE_NAME, INDEX_ATTRIBUTE_NAME, NORMAL_ATTRIBUTE_NAME, TEXCOORD_ATTRIBUTE_NAME},
                          .enableMergeDrawCalls    = false,
                          .indexType               = VK_INDEX_TYPE_UINT32,
                          .bufferAddressAble       = true,
                          .bufferForAccel          = true,
                          .bufferForStorage        = true,
                          .sceneScale              = glm::vec3(1.f)};
    loadScene(FileUtils::getResourcePath("scenes/sponza/Sponza01.gltf"));
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
    RayTracer rayTracer({});
    rayTracer.prepare();
    rayTracer.mainloop();
    return 0;
}