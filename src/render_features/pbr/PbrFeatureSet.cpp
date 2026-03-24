#include "PbrFeatureSet.h"

#include "Common/FIleUtils.h"
#include "Core/RenderContext.h"
#include "Core/Shader/GlslCompiler.h"
#include "Core/View.h"
#include "Core/Texture.h"
#include "RenderPasses/GBufferPass.h"
#include "RenderPasses/ShadowMapPass.h"
#include "Scene/SceneLoader/SceneLoaderInterface.h"
#include "engine/Gui/Gui.h"
#include "imgui.h"

namespace {
struct SkyBoxPushConstant {
    vec4  _pad0;
    float exposure;
    float gamma;
};
}

void PbrFeatureSet::initialize(Device& device) {
    GlslCompiler::forceRecompile = true;

    mForwardRenderPasses.push_back(std::make_unique<ShadowMapPass>());
    mForwardRenderPasses.push_back(std::make_unique<ForwardPass>());

    mRenderPasses.push_back(std::make_unique<GBufferPass>());
    mRenderPasses.push_back(std::make_unique<ShadowMapPass>());
    mRenderPasses.push_back(std::make_unique<IBLLightingPass>());

    for (auto& pass : mRenderPasses) {
        pass->init();
    }
    for (auto& pass : mForwardRenderPasses) {
        pass->init();
    }

    cube             = SceneLoaderInterface::loadSpecifyTypePrimitive(device, "cube");
    environmentCube  = Texture::loadTextureFromFile(g_context->getDevice(), FileUtils::getResourcePath("pisa_cube.ktx"));
    ibl              = std::make_unique<IBL>(device, environmentCube.get());
}

void PbrFeatureSet::render(RenderGraph& rg, RenderContext& renderContext, Device& device, View& view, float exposure, float gamma) {
    rg.setCutUnUsedResources(false);

    if (environmentCubeAsync) {
        environmentCube = std::move(environmentCubeAsync);
        environmentCubeAsync.reset();
        ibl->setEnvironmentCube(environmentCube.get());
    }

    ibl->importTexturesToRenderGraph(rg);
    ibl->generate(rg);

    rg.addGraphicPass(
        "Skybox",
        [&](RenderGraph::Builder& builder, GraphicPassSettings& settings) {
            auto swapchainImage = rg.getBlackBoard().getHandle(RENDER_VIEW_PORT_IMAGE_NAME);
            builder.writeTexture(swapchainImage);
            RenderGraphPassDescriptor descriptor;
            descriptor.textures = {swapchainImage};
            descriptor.addSubpass({.outputAttachments = {swapchainImage}, .disableDepthTest = true});
            builder.declare(descriptor);
        },
        [&](RenderPassContext& context) {
            renderContext.getPipelineState()
                .setDepthStencilState({.depthTestEnable = false})
                .setRasterizationState({.depthClampEnable = VK_FALSE, .cullMode = VK_CULL_MODE_NONE})
                .setPipelineLayout(device.getResourceCache().requestPipelineLayout(ShaderPipelineKey{"skybox.vert", "skybox.frag"}));
            view.bindViewBuffer();

            renderContext.bindPrimitiveGeom(context.commandBuffer, *cube)
                .bindImageSampler(0, environmentCube->getImage().getVkImageView(), environmentCube->getSampler())
                .bindPushConstants(SkyBoxPushConstant{.exposure = exposure, .gamma = gamma});
            renderContext.flushAndDrawIndexed(context.commandBuffer, cube->indexCount, 1, 0, 0, 0);
        });

    for (auto& pass : mRenderPasses) {
        pass->render(rg);
    }
}

void PbrFeatureSet::updateGui(Gui& gui) {
    for (auto& pass : mRenderPasses) {
        pass->updateGui();
    }

    auto file = gui.showFileDialog("Select a cubemap", {".ktx"});
    if (file != "no file selected") {
        ctpl::thread_pool pool(1);
        pool.push([this, file](size_t) {
            LOGI("file selected: {}", file);
            environmentCubeAsync = Texture::loadTextureFromFile(g_context->getDevice(), file);
        });
    }
}
