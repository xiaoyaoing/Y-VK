//
// Created by pc on 2023/8/17.
//

#include "RenderGraphTest.h"
#include "Core/Shader/Shader.h"
#include "../../framework/Common/VkCommon.h"
#include "Common/FIleUtils.h"
#include "Core/View.h"
#include "Scene/SceneLoader/gltfloader.h"

void Example::drawFrame(RenderGraph& rg) {

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
        view->bindViewBuffer();

        rg.addPass(
            "gbuffer", [&](RenderGraph::Builder& builder, GraphicPassSettings& settings) {
            auto albedo = rg.createTexture("albedo",
                                                     {
                                                             .extent = renderContext->getSwapChainExtent(),
                                                             .useage = TextureUsage::SUBPASS_INPUT |
                                                                       TextureUsage::COLOR_ATTACHMENT
                                                     });

            auto normal = rg.createTexture("normal",
                                                     {
                                                             .extent = renderContext->getSwapChainExtent(),
                                                             .useage = TextureUsage::SUBPASS_INPUT |
                                                                       TextureUsage::COLOR_ATTACHMENT

                                                     });

            auto depth = rg.createTexture("depth", {
                           .extent = renderContext->getSwapChainExtent(),
                           .useage = TextureUsage::SUBPASS_INPUT |
                                     TextureUsage::DEPTH_ATTACHMENT

                   });
                    
             auto output = rg.importTexture(SWAPCHAIN_IMAGE_NAME, &renderContext->getCurHwtexture());

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
            blackBoard.put(SWAPCHAIN_IMAGE_NAME, output); }, [&](RenderPassContext& context) {
            renderContext->getPipelineState().setPipelineLayout(*pipelineLayouts.gBuffer).setDepthStencilState({.depthCompareOp =  VK_COMPARE_OP_GREATER});

            for(const auto & primitive : view->getMVisiblePrimitives()) {
                renderContext->bindPrimitive(context.commandBuffer, *primitive).bindMaterial(primitive->material).flushAndDrawIndexed(commandBuffer, primitive->indexCount, 1, 0, 0, 0);
            }              
            renderContext->nextSubpass(commandBuffer);
            renderContext->getPipelineState().setPipelineLayout(*pipelineLayouts.lighting);
            renderContext->getPipelineState().setDepthStencilState({.depthTestEnable = false});

                
            renderContext->bindImage(0, blackBoard.getImageView("albedo"));
            renderContext->bindImage(1, blackBoard.getImageView("depth"));
            renderContext->bindImage(2, blackBoard.getImageView("normal"));
                
            // renderContext->bindLight<DeferredLights>(scene->getLights(), 0,4);


            renderContext->getPipelineState().setRasterizationState({
                                                                            .cullMode = VK_CULL_MODE_NONE
                                                                    });

            renderContext->flushAndDraw(commandBuffer, 3, 1, 0, 0); });
    }

    //Two RenderPass
    else {
        view->bindViewBuffer();
        rg.addPass(
            "GBufferPass", [&](RenderGraph::Builder& builder, GraphicPassSettings& settings) {
                    auto albedo = rg.createTexture("albedo",
                                                      {
                                                              .extent = renderContext->getSwapChainExtent(),
                                                              .useage = TextureUsage::SUBPASS_INPUT | 
                                                                        TextureUsage::COLOR_ATTACHMENT
                                                      });

                    auto normal = rg.createTexture("normal",
                                                      {
                                                              .extent = renderContext->getSwapChainExtent(),
                                                              .useage = TextureUsage::SUBPASS_INPUT |
                                                                        TextureUsage::COLOR_ATTACHMENT

                                                      });
                    
                    auto depth = rg.createTexture("depth", {
                            .extent = renderContext->getSwapChainExtent(),
                            .useage = TextureUsage::SUBPASS_INPUT |
                                      TextureUsage::DEPTH_ATTACHMENT

                    });

                    RenderGraphPassDescriptor desc;
                    desc.setTextures({depth, albedo,  normal}).addSubpass(RenderGraphSubpassInfo{.outputAttachments = {albedo, normal, depth}});
                    builder.declare( desc);
 

                    normal = builder.writeTexture(normal, TextureUsage::COLOR_ATTACHMENT);
                    albedo = builder.writeTexture(albedo, TextureUsage::COLOR_ATTACHMENT);
                    depth = builder.writeTexture(depth, TextureUsage::DEPTH_ATTACHMENT);


                    blackBoard.put("albedo", albedo);
                    blackBoard.put("normal", normal);
                    blackBoard.put("depth", depth); }, [&](RenderPassContext& context) {
                    //   renderContext->beginRenderPass(commandBuffer, context.renderTarget, {});
                    renderContext->getPipelineState().setPipelineLayout(*pipelineLayouts.gBuffer).setDepthStencilState({.depthCompareOp =  VK_COMPARE_OP_GREATER});
                    scene->IteratePrimitives([&](const Primitive &primitive) {
                            //todo: use camera data here
                            renderContext->bindPrimitive(context.commandBuffer,primitive).bindMaterial(primitive.material).flushAndDrawIndexed(commandBuffer, primitive.indexCount);
                                             }
                    ); });

        rg.addPass(
            "LightingPass", [&](RenderGraph::Builder& builder, GraphicPassSettings& settings) {
                auto depth  = blackBoard["depth"];
                auto normal = blackBoard["normal"];
                auto albedo = blackBoard["albedo"];
                auto output = rg.importTexture(SWAPCHAIN_IMAGE_NAME, &renderContext->getCurHwtexture());

                rg.getBlackBoard().put(SWAPCHAIN_IMAGE_NAME, output);

                output = builder.writeTexture(output, TextureUsage::COLOR_ATTACHMENT);
                output = builder.readTexture(output, TextureUsage::COLOR_ATTACHMENT);

                normal = builder.readTexture(normal);
                albedo = builder.readTexture(albedo);
                depth  = builder.readTexture(depth);

                RenderGraphPassDescriptor desc{};
                desc.setTextures({output, albedo, depth, normal}).addSubpass({.inputAttachments = {albedo, depth, normal}, .outputAttachments = {output}, .disableDepthTest = true});
                builder.declare(desc);
                // builder.addSubPass();
            },
            [&](RenderPassContext& context) {
                renderContext->getPipelineState().setPipelineLayout(*pipelineLayouts.lighting).setRasterizationState({.cullMode = VK_CULL_MODE_NONE}).setDepthStencilState({.depthTestEnable = false});

                renderContext->bindImage(0, blackBoard.getImageView("albedo"))
                    .bindImage(1, blackBoard.getImageView("depth"))
                    .bindImage(2, blackBoard.getImageView("normal"))
                    .flushAndDraw(commandBuffer, 3, 1, 0, 0);
            });
    }

    //rg.clearPass();

    mCurrentTextures = rg.getResourceNames(RENDER_GRAPH_RESOURCE_TYPE::ETexture);

    rg.addImageCopyPass(blackBoard.getHandle(mPresentTexture), blackBoard.getHandle(SWAPCHAIN_IMAGE_NAME));
    // rg.addImageCopyPass(blackBoard.getHandle("normal"), blackBoard.getHandle(SWAPCHAIN_IMAGE_NAME));

    gui->addGuiPass(rg);

    rg.compile();

    rg.execute(commandBuffer);
}

void Example::prepare() {
    Application::prepare();

    std::vector<Shader> shaders{
        Shader(*device, FileUtils::getShaderPath("defered.vert")),
        Shader(*device, FileUtils::getShaderPath("defered.frag"))};
    pipelineLayouts.gBuffer = std::make_unique<PipelineLayout>(*device, shaders);

    std::vector<Shader> shaders1{
        Shader(*device, FileUtils::getShaderPath("lighting.vert")),
        Shader(*device, FileUtils::getShaderPath("lighting.frag"))};
    pipelineLayouts.lighting = std::make_unique<PipelineLayout>(*device, shaders1);

    // scene = GltfLoading::LoadSceneFromGLTFFile(*device, FileUtils::getResourcePath("space_module/SpaceModule.gltf"));
    scene = GltfLoading::LoadSceneFromGLTFFile(*device, FileUtils::getResourcePath("sponza/Sponza01.gltf"));
    // scene = GltfLoading::LoadSceneFromGLTFFile(*device, "E:/code/DirectX-Graphics-Samples/MiniEngine/ModelViewer/Sponza/pbr/sponza2.gltf");
    // scene = GltfLoading::LoadSceneFromGLTFFile(*device, "E:/code/DirectX-Graphics-Samples/MiniEngine/ModelViewer/Sponza/pbr/sponza2.gltf");
    //  scene = GltfLoading::LoadSceneFromGLTFFile(*device, FileUtils::getResourcePath("cornell-box/cornellBox.gltf"));

    auto light_pos   = glm::vec3(0.0f, 128.0f, -225.0f);
    auto light_color = glm::vec3(1.0, 1.0, 1.0);

    // Magic numbers used to offset lights in the Sponza scene
    for (int i = -4; i < 4; ++i) {
        for (int j = 0; j < 2; ++j) {
            glm::vec3 pos = light_pos;
            pos.x += i * 400;
            pos.z += j * (225 + 140);
            pos.y = 8;

            for (int k = 0; k < 3; ++k) {
                pos.y = pos.y + (k * 100);

                light_color.x = static_cast<float>(rand()) / (RAND_MAX);
                light_color.y = static_cast<float>(rand()) / (RAND_MAX);
                light_color.z = static_cast<float>(rand()) / (RAND_MAX);

                LightProperties props;
                props.color     = light_color;
                props.intensity = 0.2f;
                props.position  = pos;

                scene->addLight(SgLight{.type = LIGHT_TYPE::Point, .lightProperties = props});
            }
        }
    }
    camera        = scene->getCameras()[0];
    camera->flipY = true;
    camera->setTranslation(glm::vec3(-494.f, -116.f, 99.f));
    camera->setRotation(glm::vec3(0.0f, -90.0f, 0.0f));
    camera->setRotation(glm::vec3(0.0f, -90.0f, 0.0f));
    camera->setPerspective(60.0f, (float)mWidth / (float)mHeight, 1.f, 4000.f);
    camera->setMoveSpeed(0.05f);

    view = std::make_unique<View>(*device);
    view->setScene(scene.get());
    view->setCamera(camera.get());
}

Example::Example() : Application("Drawing Triangle", 1024, 1024) {
    addDeviceExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
}

void Example::onUpdateGUI() {
    gui->checkBox("Use subpasses", &useSubpass);

    auto itemIter    = std::ranges::find(mCurrentTextures.begin(), mCurrentTextures.end(), mPresentTexture);
    int  itemCurrent = itemIter - mCurrentTextures.begin();

    ImGui::Combo("RenderGraphTextures", &itemCurrent, mCurrentTextures.data(), mCurrentTextures.size());
    mPresentTexture = mCurrentTextures[itemCurrent];
    // ImGui::RadioButton("use subpass", &useSubpass, 1);
}

int main() {
    auto example = new Example();
    example->prepare();
    example->mainloop();
    return 0;
}