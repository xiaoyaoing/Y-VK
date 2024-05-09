//
// Created by pc on 2023/8/17.
//

#include "RenderGraphTest.h"
#include "Core/Shader/Shader.h"
#include "../../framework/Common/VkCommon.h"
#include "Common/FIleUtils.h"
#include "Core/View.h"
#include "Core/math.h"
#include "Core/Shader/GlslCompiler.h"
#include "RenderPasses/GBufferPass.h"
#include "Scene/SceneLoader/SceneLoaderInterface.h"
#include "Scene/SceneLoader/gltfloader.h"

void Example::drawFrame(RenderGraph& rg) {
    
    for(auto & pass : passes) {
        pass->render(rg);
    }
    
    return ;

    auto& commandBuffer = renderContext->getGraphicCommandBuffer();
    
    auto& blackBoard = rg.getBlackBoard();
    
    //ON PASS TWO SUBPASS
    struct OnePassTwoSubPassDeferedShadingData {
    
        RenderGraphHandle albedo;
    
        RenderGraphHandle normal;
    
        RenderGraphHandle depth;
    
        RenderGraphHandle output;
    
        RENDER_GRAPH_PASS_TYPE type = RENDER_GRAPH_PASS_TYPE::GRAPHICS;
    };
    
    if (useSubpass) {
    
        rg.addGraphicPass(
            "gbuffer", [&](RenderGraph::Builder& builder, GraphicPassSettings& settings) {
            auto albedo = rg.createTexture("albedo",
                                                     {
                                                             .extent = renderContext->getViewPortExtent(),
                                                             .useage = TextureUsage::SUBPASS_INPUT |
                                                                       TextureUsage::COLOR_ATTACHMENT
                                                     });
    
            auto normal = rg.createTexture("normal",
                                                     {
                                                             .extent = renderContext->getViewPortExtent(),
                                                             .useage = TextureUsage::SUBPASS_INPUT |
                                                                       TextureUsage::COLOR_ATTACHMENT
    
                                                     });
    
            auto depth = rg.createTexture("depth", {
                           .extent = renderContext->getViewPortExtent(),
                           .useage = TextureUsage::SUBPASS_INPUT |
                                     TextureUsage::DEPTH_ATTACHMENT
    
                   });
                    
             auto output = rg.getBlackBoard().getHandle(RENDER_VIEW_PORT_IMAGE_NAME);
    
            RenderGraphPassDescriptor desc;
            desc.setTextures({output, depth, albedo,normal}).addSubpass({.outputAttachments = {albedo, normal, depth}}).addSubpass({
                                            .inputAttachments = {
                                                    albedo, depth,
                                                    normal
                                            },
                                            .outputAttachments = {output}
                                    });
            builder.declare( desc);
            blackBoard.put("albedo", albedo);
            blackBoard.put("normal", normal);
            blackBoard.put("depth", depth);
            blackBoard.put(RENDER_VIEW_PORT_IMAGE_NAME, output); }, [&](RenderPassContext& context) {
    
            view->bindViewBuffer().bindViewShading();
    
            renderContext->bindScene(commandBuffer,*scene).getPipelineState().setPipelineLayout(*pipelineLayouts.gBuffer).setDepthStencilState({.depthCompareOp =  VK_COMPARE_OP_GREATER});
    
            uint32_t instance_count= 0;    
            for(const auto & primitive : view->getMVisiblePrimitives()) {
                renderContext->flushAndDrawIndexed(commandBuffer, primitive->indexCount, 1, primitive->firstIndex, primitive->firstVertex,instance_count++);
            }              
            renderContext->nextSubpass(commandBuffer);
            renderContext->getPipelineState().setPipelineLayout(*pipelineLayouts.lighting);
            renderContext->getPipelineState().setDepthStencilState({.depthTestEnable = false});
    
                
            renderContext->bindImage(0, blackBoard.getImageView("albedo")).bindImage(1, blackBoard.getImageView("depth")).bindImage(2, blackBoard.getImageView("normal"));
                
            renderContext->getPipelineState().setRasterizationState({
                                                                            .cullMode = VK_CULL_MODE_NONE
                                                                    });
            renderContext->flushAndDraw(commandBuffer, 3, 1, 0, 0); });
    }

  
}

void Example::prepare() {
    Application::prepare();

    // std::vector<Shader> shaders{
    //     Shader(*device, FileUtils::getShaderPath("defered_one_scene_buffer.vert")),
    //     Shader(*device, FileUtils::getShaderPath("defered.frag"))};
    // pipelineLayouts.gBuffer = std::make_unique<PipelineLayout>(*device, shaders);
    //
    // std::vector<Shader> shaders1{
    //     Shader(*device, FileUtils::getShaderPath("lighting.vert")),
    //     Shader(*device, FileUtils::getShaderPath("lighting.frag"))};
    // pipelineLayouts.lighting = std::make_unique<PipelineLayout>(*device, shaders1);

    passes.emplace_back(std::make_unique<GBufferPass>());
    passes.emplace_back(std::make_unique<LightingPass>());

    for(auto & pass : passes) {
        pass->init();
    }

    SceneLoadingConfig config;
    
    // scene = SceneLoaderInterface::LoadSceneFromFile(*device, "E:/code/VulkanFrameWorkLearn/resources/sponza/Sponza01.gltf", {.bufferRate = BufferRate::PER_SCENE});
    scene = SceneLoaderInterface::LoadSceneFromFile(*device, "E:/code/MoerEngineScenes/Sponza/pbr/sponza2.gltf", {.bufferRate = BufferRate::PER_SCENE});

    SceneLoadingConfig sceneConfig = {.requiredVertexAttribute = {POSITION_ATTRIBUTE_NAME, INDEX_ATTRIBUTE_NAME, NORMAL_ATTRIBUTE_NAME, TEXCOORD_ATTRIBUTE_NAME},
                                      .indexType               = VK_INDEX_TYPE_UINT32,
                                      .bufferAddressAble       = true,
                                      .bufferForAccel          = true,
                                      .bufferForStorage        = true,
                                      .sceneScale = glm::vec3(0.1f)};
    
    auto light_pos   = glm::vec3(0.0f, 128.0f, -225.0f);
    auto light_color = glm::vec3(1.0, 1.0, 1.0);

    // Magic numbers used to offset lights in the Sponza scene
    // for (int i = -4; i < 4; ++i) {
    //     for (int j = 0; j < 2; ++j) {
    //         glm::vec3 pos = light_pos;
    //         pos.x += i * 400;
    //         pos.z += j * (225 + 140);
    //         pos.y = 8;
    //
    //         for (int k = 0; k < 3; ++k) {
    //             pos.y = pos.y + (k * 100);
    //
    //             light_color.x = static_cast<float>(rand()) / (RAND_MAX);
    //             light_color.y = static_cast<float>(rand()) / (RAND_MAX);
    //             light_color.z = static_cast<float>(rand()) / (RAND_MAX);
    //
    //             LightProperties props;
    //             props.color     = light_color;
    //             props.intensity = 0.2f;
    //             props.position  = pos;
    //
    //             scene->addLight(SgLight{.type = LIGHT_TYPE::Point, .lightProperties = props});
    //         }
    //     }
    // }
    scene->addDirectionalLight({0, -0.95f, 0.3f}, glm::vec3(1.0f), 1.5f);

    
    camera = scene->getCameras()[0];
    camera->setRotation(glm::vec3(0.0f, -90.0f, 0.0f));
    camera->setPerspective(60.0f, (float)mWidth / (float)mHeight, 1.f, 4000.f);
    camera->setMoveSpeed(0.0005f);

    camera->getTransform()->setPosition(glm::vec3(0, 1, 4));
    camera->getTransform()->setRotation(glm::quat(1, 0, 0, 0));

    view = std::make_unique<View>(*device);
    view->setScene(scene.get());
    view->setCamera(camera.get());

    RenderPtrManangr::init();
    g_manager->putPtr("view", view.get());

}

Example::Example() : Application("Defered Rendering Sponza", 1920, 1080) {
    addDeviceExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    GlslCompiler::forceRecompile = true;
}

void Example::onUpdateGUI() {
    gui->checkBox("Use subpasses", &useSubpass);
}

int main() {
    auto example = new Example();
    example->prepare();
    example->mainloop();
    return 0;
}