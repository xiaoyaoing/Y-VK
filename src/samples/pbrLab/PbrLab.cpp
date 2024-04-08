//
// Created by pc on 2023/8/17.
//

#include "PbrLab.h"
#include "Core/Shader/Shader.h"
#include "../../framework/Common/VkCommon.h"
#include "Common/FIleUtils.h"
#include "Core/View.h"
#include "Core/Shader/GlslCompiler.h"
#include "RenderPasses/GBufferPass.h"
#include "Scene/SceneLoader/gltfloader.h"

void Example::drawFrame(RenderGraph& rg) {
    rg.setCutUnUsedResources(false);
    auto& commandBuffer = renderContext->getGraphicCommandBuffer();

    auto& blackBoard = rg.getBlackBoard();

    for (auto& pass : mRenderPasses) {
        pass->render(rg);
    }
    gui->addGuiPass(rg);

    rg.execute(commandBuffer);
}

void Example::prepare() {

    Application::prepare();

    mRenderPasses.push_back(std::make_unique<GBufferPass>());
    mRenderPasses.push_back(std::make_unique<LightingPass>());

    scene = GltfLoading::LoadSceneFromGLTFFile(*device, "E:/code/Vulkan-glTF-PBR/data/models/DamagedHelmet/glTF-Embedded/DamagedHelmet.gltf");

    scene->addDirectionalLight({0, -0.95f, 0.3f}, glm::vec3(1.0f), 1.5f);
    scene->addDirectionalLight({1.0f, 0, 0}, glm::vec3(1.0f), 0.5f);

    camera        = scene->getCameras()[0];
    camera->flipY = true;
    camera->setTranslation(glm::vec3(0, 1.35, -5));
    camera->setRotation(glm::vec3(0.0f, 0, 0.0f));
    camera->setPerspective(60.0f, (float)mWidth / (float)mHeight, 1.f, 4000.f);
    camera->setMoveSpeed(0.05f);

    view = std::make_unique<View>(*device);
    view->setScene(scene.get());
    view->setCamera(camera.get());

    RenderPtrManangr::init();
    g_manager->putPtr("view", view.get());

    for (auto& pass : mRenderPasses) {
        pass->init();
    }
}

Example::Example() : Application("Pbr Lab", 1920, 1024) {
    addDeviceExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    GlslCompiler::forceRecompile = true;
}

void Example::onUpdateGUI() {
    // ImGui::Selectable()
}

int main() {
    auto example = new Example();
    example->prepare();
    example->mainloop();
    return 0;
}