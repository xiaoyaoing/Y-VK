//
// Created by pc on 2023/8/17.
//

#include "PbrLab.h"
#include "PbrLab.h"
#include "Core/Shader/Shader.h"
#include "../../framework/Common/VkCommon.h"
#include "Common/FIleUtils.h"
#include "Common/ResourceCache.h"
#include "Core/View.h"
#include "Core/math.h"
#include "Core/Shader/GlslCompiler.h"
#include "RenderPasses/GBufferPass.h"
#include "Scene/SceneLoader/SceneLoaderInterface.h"

struct SkyBoxPushConstant {
    vec4  _pad0;
    float exposure;
    float gamma;
};

void Example::drawFrame(RenderGraph& rg) {
    rg.setCutUnUsedResources(false);

    ibl->importTexturesToRenderGraph(rg);
    ibl->generate(rg);

    rg.addGraphicPass(
        "",
        [&](RenderGraph::Builder& builder, GraphicPassSettings& settings) {
            auto swapchainImage = rg.getBlackBoard().getHandle(SWAPCHAIN_IMAGE_NAME);
            builder.writeTexture(swapchainImage);
            RenderGraphPassDescriptor descriptor;
            descriptor.textures = {swapchainImage};
            descriptor.addSubpass({.outputAttachments = {swapchainImage}});
            builder.declare(descriptor);
        },
        [&](RenderPassContext& context) {
            renderContext->getPipelineState().setDepthStencilState({.depthTestEnable = false}).setRasterizationState({.cullMode = VK_CULL_MODE_NONE}).setPipelineLayout(device->getResourceCache().requestPipelineLayout(std::vector<std::string>{"skybox.vert", "skybox.frag"}));
            view->bindViewBuffer();
            renderContext->bindPrimitiveGeom(context.commandBuffer, *cube).bindImageSampler(0, environmentCube->getImage().getVkImageView(), environmentCube->getSampler()).bindPushConstants(SkyBoxPushConstant{.exposure = exposure, .gamma = gamma});
            renderContext->flushAndDrawIndexed(context.commandBuffer, cube->indexCount, 1, 0, 0, 0);
        });

    for (auto& pass : mRenderPasses) {
        pass->render(rg);
    }
}

void Example::prepare() {
    Application::prepare();

    g_context->setFlipViewport(true);
    mRenderPasses.push_back(std::make_unique<GBufferPass>());
    //  mRenderPasses.push_back(std::make_unique<LightingPass>());
    mRenderPasses.push_back(std::make_unique<IBLLightingPass>());

    float              matrix[9] = {1, 0, 0, 0, -1, 0, 0, 0, 1};
    glm::mat3          rotate    = glm::make_mat3(matrix);
    SceneLoadingConfig config{.sceneRotation = glm::quat_cast(rotate)};
    config = {};
    auto m = glm::toMat3(config.sceneRotation);
    // config.sceneRotation = glm::quat(0, 0, 0, 1);
    scene = SceneLoaderInterface::LoadSceneFromFile(*device, "E:/code/Vulkan-glTF-PBR/data/models/DamagedHelmet/glTF-Embedded/DamagedHelmet.gltf", config);

    scene->addDirectionalLight({0, -0.95f, 0.3f}, glm::vec3(1.0f), 1.5f);
    scene->addDirectionalLight({1.0f, 0, 0}, glm::vec3(1.0f), 0.5f);

    camera        = scene->getCameras()[0];
    camera->flipY = true;
    // camera->setTranslation(glm::vec3(0, 1.35, -5));
    camera->setTranslation(glm::vec3(0, 0, 4));
    camera->setRotation(glm::vec3(0.0f, 0, 0.0f));
    camera->getTransform()->setRotation(glm::quat(1, 0, 0, 0));
    camera->setPerspective(60.0f, (float)mWidth / (float)mHeight, 0.1f, 4000.f);
    camera->setMoveSpeed(0.05f);

    view = std::make_unique<View>(*device);
    view->setScene(scene.get());
    view->setCamera(camera.get());

    RenderPtrManangr::init();
    g_manager->putPtr("view", view.get());

    for (auto& pass : mRenderPasses) {
        pass->init();
    }

    cube             = SceneLoaderInterface::loadSpecifyTypePrimitive(*device, "cube");
    std::string path = "E:/code/Vulkan-glTF-PBR/data/environments/papermill.ktx";
    path             = "E:/code/Vulkan-Samples/assets/textures/uffizi_rgba16f_cube_inverse.ktx";
    environmentCube  = Texture::loadTextureFromFile(g_context->getDevice(), path);
    ibl              = std::make_unique<IBL>(*device, environmentCube.get());
}

Example::Example() : Application("Pbr Lab", 1920, 1024) {
    addDeviceExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    // GlslCompiler::forceRecompile = true;
}

void Example::onUpdateGUI() {
    for (auto& pass : mRenderPasses) {
        pass->updateGui();
    }
    // ImGui::Selectable()
}

int main() {
    auto example = new Example();
    example->prepare();
    example->mainloop();
    return 0;
}