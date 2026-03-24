#include "PbrRenderer.h"

#include "app/editor/EditorApplication.h"

#include "Common/FIleUtils.h"
#include "Common/ResourceCache.h"
#include "Core/Shader/GlslCompiler.h"
#include "Core/View.h"
#include "RenderPasses/GBufferPass.h"
#include "RenderPasses/ShadowMapPass.h"
#include "Scene/SceneLoader/SceneLoaderInterface.h"
#include "ctpl_stl.h"

struct SkyBoxPushConstant {
    vec4 _pad0;
    float exposure;
    float gamma;
};

void PbrRenderer::initialize(EditorApplication& editor) {
    if (mInitialized) {
        return;
    }

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

    cube = SceneLoaderInterface::loadSpecifyTypePrimitive(editor.getDevice(), "cube");
    environmentCube = Texture::loadTextureFromFile(editor.getDevice(), FileUtils::getResourcePath("pisa_cube.ktx"));
    ibl = std::make_unique<IBL>(editor.getDevice(), environmentCube.get());
    mInitialized = true;
}

void PbrRenderer::onActivated(EditorApplication&) {
    g_context->setFlipViewport(true);
}

void PbrRenderer::onSceneLoaded(EditorApplication& editor) {
    if (editor.getScene()) {
        editor.getScene()->addDirectionalLight({0, -0.5f, -0.12f}, glm::vec3(1.0f), 1.5f, vec3(0, 50, 0));
    }
    editor.finalizeSceneLoaded();
}

void PbrRenderer::render(EditorApplication& editor, RenderGraph& rg) {
    rg.setCutUnUsedResources(false);

    if (environmentCubeAsync) {
        environmentCube = std::move(environmentCubeAsync);
        ibl->setEnvironmentCube(environmentCube.get());
    }

    ibl->importTexturesToRenderGraph(rg);
    ibl->generate(rg);

    rg.addGraphicPass(
        "",
        [&](RenderGraph::Builder& builder, GraphicPassSettings& settings) {
            auto swapchainImage = rg.getBlackBoard().getHandle(RENDER_VIEW_PORT_IMAGE_NAME);
            builder.writeTexture(swapchainImage);
            RenderGraphPassDescriptor descriptor;
            descriptor.textures = {swapchainImage};
            descriptor.addSubpass({.outputAttachments = {swapchainImage}, .disableDepthTest = true});
            builder.declare(descriptor);
        },
        [&](RenderPassContext& context) {
            auto* view = editor.getView();
            auto& renderContext = editor.getRenderContext();
            auto& device = editor.getDevice();
            renderContext.getPipelineState()
                .setDepthStencilState({.depthTestEnable = false})
                .setRasterizationState({.depthClampEnable = VK_FALSE, .cullMode = VK_CULL_MODE_NONE})
                .setPipelineLayout(device.getResourceCache().requestPipelineLayout(ShaderPipelineKey{"skybox.vert", "skybox.frag"}));
            view->bindViewBuffer();

            renderContext.bindPrimitiveGeom(context.commandBuffer, *cube)
                .bindImageSampler(0, environmentCube->getImage().getVkImageView(), environmentCube->getSampler())
                .bindPushConstants(SkyBoxPushConstant{.exposure = exposure, .gamma = gamma});
            renderContext.flushAndDrawIndexed(context.commandBuffer, cube->indexCount, 1, 0, 0, 0);
        });

    for (auto& pass : mRenderPasses) {
        pass->render(rg);
    }
}

void PbrRenderer::updateGui(EditorApplication& editor) {
    for (auto& pass : mRenderPasses) {
        pass->updateGui();
    }

    auto* gui = editor.getGui();
    auto file = gui->showFileDialog("Select a cubemap", {".ktx"});
    if (file != "no file selected") {
        ctpl::thread_pool pool(1);
        pool.push([this, file](size_t) {
            environmentCubeAsync = Texture::loadTextureFromFile(g_context->getDevice(), file);
        });
    }
}
