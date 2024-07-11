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
#include "RenderPasses/SSGIPass.h"
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
            auto swapchainImage = rg.getBlackBoard().getHandle(RENDER_VIEW_PORT_IMAGE_NAME);
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
    GlslCompiler::forceRecompile = true;
    Application::prepare();

    g_context->setFlipViewport(true);
    mRenderPasses.push_back(std::make_unique<GBufferPass>());
    mRenderPasses.push_back(std::make_unique<LightingPass>());
    //mRenderPasses.push_back(std::make_unique<IBLLightingPass>());
    //mRenderPasses.push_back(std::make_unique<SSGIPass>());

    // loadScene("E:/code/Vulkan-glTF-PBR/data/models/DamagedHelmet/glTF-Embedded/DamagedHelmet.gltf");
   // loadScene("E:/code/FidelityFX-SSSR/sample/media/Chess/scene.gltf");
    //loadScene("C://Users//yjp//Downloads//ABeautifulGame//glTF//ABeautifulGame.gltf");
    loadScene("C:/Users/yuanjunping/Downloads/teapot/scene.json");


    RenderPtrManangr::init();
    g_manager->putPtr("view", view.get());

    for (auto& pass : mRenderPasses) {
        pass->init();
    }

    cube             = SceneLoaderInterface::loadSpecifyTypePrimitive(*device, "cube");
    std::string path = "C:/Users/yuanjunping/Downloads/papermill.ktx";
    environmentCube  = Texture::loadTextureFromFile(g_context->getDevice(), path);
    ibl              = std::make_unique<IBL>(*device, environmentCube.get());
}

Example::Example() : Application("Pbr Lab", 1920, 1080) {
    addDeviceExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
}

void Example::onUpdateGUI() {
    for (auto& pass : mRenderPasses) {
        pass->updateGui();
    }
    //vec3 & dir = scene->getLights1()[1].lightProperties.direction;
    ImGui::SliderFloat("dir x", &dir.x, -1, 1);
    ImGui::SliderFloat("dir y", &dir.y, -1, 1);
    ImGui::SliderFloat("dir z", &dir.z, -1, 1);
    ImGui::SliderFloat("light intensity", &scene->getLights1()[1].lightProperties.intensity, 0, 100);
    
    scene->getLights1()[1].lightProperties.direction = dir;
    
    // ImGui::Selectable()
}

int main() {
    auto example = new Example();
    example->prepare();
    example->mainloop();
    return 0;
}