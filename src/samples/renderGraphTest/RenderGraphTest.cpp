//
// Created by pc on 2023/8/17.
//

#include "RenderGraphTest.h"
#include "Core/Shader/Shader.h"
#include "../../framework/Common/VkCommon.h"
#include "Common/FIleUtils.h"

void Example::drawFrame(RenderGraph & rg,CommandBuffer &commandBuffer) {
   
    auto &blackBoard = rg.getBlackBoard();

    //ON PASS TWO SUBPASS
    struct OnePassTwoSubPassDeferedShadingData {

        RenderGraphHandle albedo;

        RenderGraphHandle normal;

        RenderGraphHandle depth;


        RenderGraphHandle output;

            RENDER_GRAPH_PASS_TYPE type = GRAPHICS;

    };

    if (useSubpass) {

        rg.addPass("gbuffer",[&](RenderGraph::Builder &builder,GraphicPassSettings & settings)
        {
            auto albedo = rg.createTexture("color",
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
                    
             auto output = rg.importTexture("output", &renderContext->getCurHwtexture());

            RenderGraphPassDescriptor desc{
                            .textures = {
                                    output, depth, albedo,
                                    normal
                            }
                    };
            desc.addSubpass({.outputAttachments = {albedo, normal, depth}}).addSubpass({
                                            .inputAttachments = {
                                                    albedo, depth,
                                                    normal
                                            },
                                            .outputAttachments = {output}
                                    });
            builder.declare("Color Pass Target", desc);

                blackBoard.put("albedo", albedo);
                    blackBoard.put("normal", normal);
                    blackBoard.put("depth", depth);
                    blackBoard.put("output", output);
        },
        [&](RenderPassContext & context)   {
            renderContext->getPipelineState().setPipelineLayout(*pipelineLayouts.gBuffer).setDepthStencilState({.depthCompareOp =  VK_COMPARE_OP_GREATER});
                
            scene->IteratePrimitives([&](const Primitive &primitive) {
                                                 renderContext->bindPrimitive(primitive);

                                                 const auto allocation = renderContext->allocateBuffer(
                                                         sizeof(GlobalUniform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

                                                 auto view = camera->matrices.view;
                                                 //todo: use camera data here
                                                 GlobalUniform uniform{
                                                         .model = primitive.matrix, .view = view,
                                                         .proj = camera->matrices.perspective
                                                 };
                                                 allocation.buffer->uploadData(
                                                         &uniform, allocation.size, allocation.offset);

                                                 renderContext->bindBuffer(
                                                         0, *allocation.buffer, allocation.offset, allocation.size);
                                                 renderContext->bindMaterial(primitive.material);
                                                 renderContext->flushAndDrawIndexed(
                                                         commandBuffer, primitive.indexCount, 1, 0, 0, 0);
                                             }
                    );
                    renderContext->nextSubpass(commandBuffer);
                    renderContext->getPipelineState().setPipelineLayout(*pipelineLayouts.lighting);
                    renderContext->getPipelineState().setDepthStencilState({.depthTestEnable = false});
                        
                    struct {
                        glm::mat4 invView;
                        glm::vec2 invRes;
                    } fragUniform;
                    fragUniform.invRes = glm::vec2(1.0f / renderContext->getSwapChainExtent().width,
                                                   1.0f / renderContext->getSwapChainExtent().height);
                    fragUniform.invView = glm::inverse(camera->matrices.perspective * camera->matrices.view);

                    auto viewBuffer = renderContext->
                            allocateBuffer(sizeof(fragUniform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
                    viewBuffer.buffer->uploadData(&fragUniform, viewBuffer.size, viewBuffer.offset);
                    renderContext->bindBuffer(3, *viewBuffer.buffer, viewBuffer.offset, viewBuffer.size);

                        
                    renderContext->bindInput(0, blackBoard.getImageView("albedo"), 0, 0);
                    renderContext->bindInput(0, blackBoard.getImageView("depth"), 1, 0);
                    renderContext->bindInput(0, blackBoard.getImageView("normal"), 2, 0);
                        
                    renderContext->bindLight<DeferredLights>(scene->getLights(), 0, 4);


                    renderContext->getPipelineState().setRasterizationState({
                                                                                    .cullMode = VK_CULL_MODE_NONE
                                                                            });

                    renderContext->flushAndDraw(commandBuffer, 3, 1, 0, 0);
        });
        if(false )
        rg.addPass(
                "gbuffer", [&](RenderGraph::Builder &builder,GraphicPassSettings & settings) {
                    auto albedo = rg.createTexture("color",
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
                    
                    auto output = rg.importTexture("output", &renderContext->getCurHwtexture());


                    blackBoard.put("albedo", albedo);
                    blackBoard.put("normal", normal);
                    blackBoard.put("depth", depth);
                    blackBoard.put("output", output);

                    RenderGraphPassDescriptor desc{
                            .textures = {
                                    output, depth, albedo,
                                    normal
                            }
                    };
                    desc.addSubpass({.outputAttachments = {albedo, normal, depth}}).addSubpass({
                                            .inputAttachments = {
                                                    albedo, depth,
                                                    normal
                                            },
                                            .outputAttachments = {output}
                                    });

                    builder.declare("Color Pass Target", desc);

                    output = builder.writeTexture(output, TextureUsage::COLOR_ATTACHMENT);
                    depth = builder.writeTexture(depth, TextureUsage::COLOR_ATTACHMENT);
                    normal = builder.writeTexture(
                            normal, TextureUsage::COLOR_ATTACHMENT);
                    albedo = builder.writeTexture(
                            albedo, TextureUsage::COLOR_ATTACHMENT);
                    depth = builder.writeTexture(depth, TextureUsage::DEPTH_ATTACHMENT);

                    output = builder.readTexture(output, TextureUsage::COLOR_ATTACHMENT);
                    depth = builder.readTexture(
                            depth, TextureUsage::DEPTH_ATTACHMENT);
                    normal = builder.readTexture(
                            normal, TextureUsage::COLOR_ATTACHMENT);
                    albedo = builder.readTexture(
                            albedo, TextureUsage::COLOR_ATTACHMENT);
                    depth = builder.readTexture(depth, TextureUsage::DEPTH_ATTACHMENT);
                },
                [&](RenderPassContext & context) {
                    renderContext->getPipelineState().setPipelineLayout(*pipelineLayouts.gBuffer).setDepthStencilState({.depthCompareOp =  VK_COMPARE_OP_GREATER});
                    


                    scene->IteratePrimitives([&](const Primitive &primitive) {
                                                 renderContext->bindPrimitive(primitive);

                                                 const auto allocation = renderContext->allocateBuffer(
                                                         sizeof(GlobalUniform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

                                                 auto view = camera->matrices.view;
                                                 //todo: use camera data here
                                                 GlobalUniform uniform{
                                                         .model = primitive.matrix, .view = view,
                                                         .proj = camera->matrices.perspective
                                                 };
                                                 allocation.buffer->uploadData(
                                                         &uniform, allocation.size, allocation.offset);

                                                 renderContext->bindBuffer(
                                                         0, *allocation.buffer, allocation.offset, allocation.size);
                                                 renderContext->bindMaterial(primitive.material);
                                                 renderContext->flushAndDrawIndexed(
                                                         commandBuffer, primitive.indexCount, 1, 0, 0, 0);
                                             }
                    );
                    renderContext->nextSubpass(commandBuffer);
                    renderContext->getPipelineState().setPipelineLayout(*pipelineLayouts.lighting);
                    renderContext->getPipelineState().setDepthStencilState({.depthTestEnable = false});
                        
                    struct {
                        glm::mat4 invView;
                        glm::vec2 invRes;
                    } fragUniform;
                    fragUniform.invRes = glm::vec2(1.0f / renderContext->getSwapChainExtent().width,
                                                   1.0f / renderContext->getSwapChainExtent().height);
                    fragUniform.invView = glm::inverse(camera->matrices.perspective * camera->matrices.view);

                    auto viewBuffer = renderContext->
                            allocateBuffer(sizeof(fragUniform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
                    viewBuffer.buffer->uploadData(&fragUniform, viewBuffer.size, viewBuffer.offset);
                    renderContext->bindBuffer(3, *viewBuffer.buffer, viewBuffer.offset, viewBuffer.size);

                        
                    renderContext->bindInput(0, blackBoard.getImageView("albedo"), 0, 0);
                    renderContext->bindInput(0, blackBoard.getImageView("depth"), 1, 0);
                    renderContext->bindInput(0, blackBoard.getImageView("normal"), 2, 0);
                        
                    renderContext->bindLight<DeferredLights>(scene->getLights(), 0, 4);


                    renderContext->getPipelineState().setRasterizationState({
                                                                                    .cullMode = VK_CULL_MODE_NONE
                                                                            });

                    renderContext->flushAndDraw(commandBuffer, 3, 1, 0, 0);


                });
    }


        //Two RenderPass
    else {
        struct GBufferData {
            RenderGraphHandle albedo;
            RenderGraphHandle normal;
            RenderGraphHandle depth;
            RENDER_GRAPH_PASS_TYPE type = GRAPHICS;
        };
        rg.addPass(
                "GBufferPass", [&](RenderGraph::Builder &builder,GraphicPassSettings & settings) {
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

                    RenderGraphPassDescriptor desc{.textures = {depth, albedo, depth, normal}};
                    builder.declare("GBuffer", desc);


                    normal = builder.writeTexture(normal, TextureUsage::COLOR_ATTACHMENT);
                    albedo = builder.writeTexture(albedo, TextureUsage::COLOR_ATTACHMENT);
                    depth = builder.writeTexture(depth, TextureUsage::COLOR_ATTACHMENT);
                    depth = builder.writeTexture(depth, TextureUsage::DEPTH_ATTACHMENT);
                    depth = builder.readTexture(depth, TextureUsage::DEPTH_ATTACHMENT);


                    blackBoard.put("albedo", albedo);
                    blackBoard.put("normal", normal);
                    blackBoard.put("depth", depth);
                },
                [&](RenderPassContext & context) {
                    //   renderContext->beginRenderPass(commandBuffer, context.renderTarget, {});
                    renderContext->getPipelineState().setPipelineLayout(*pipelineLayouts.gBuffer).setDepthStencilState({.depthCompareOp =  VK_COMPARE_OP_GREATER});
                    scene->IteratePrimitives([&](const Primitive &primitive) {
                            const auto allocation = renderContext->allocateBuffer(
                                    sizeof(GlobalUniform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

                            //todo: use camera data here
                            GlobalUniform uniform{
                                    .model = primitive.matrix, .view = camera->matrices.view,
                                    .proj = camera->matrices.perspective
                            };
                            allocation.buffer->uploadData(&uniform, allocation.size, allocation.offset);

                            renderContext->bindBuffer(0, *allocation.buffer, allocation.offset, allocation.size, 0,0)
                                        .bindPrimitive(primitive)
                                        .flushAndDrawIndexed(commandBuffer, primitive.indexCount, 1, 0, 0, 0);
                                             }
                    );
                });

        struct LightingData {
            RenderGraphHandle depth;
            RenderGraphHandle albedo;
            RenderGraphHandle normal;
            RenderGraphHandle output;
                RENDER_GRAPH_PASS_TYPE type = GRAPHICS;

        };

        rg.addPass(
                "LightingPass", [&](RenderGraph::Builder &builder,GraphicPassSettings & settings) {
                    auto depth = blackBoard["depth"];
                    auto normal = blackBoard["normal"];
                    auto albedo = blackBoard["albedo"];
                    auto output = rg.importTexture("output", &renderContext->getCurHwtexture());

                    rg.getBlackBoard().put("output", output);

                    output = builder.writeTexture(output, TextureUsage::COLOR_ATTACHMENT);
                    output = builder.readTexture(output, TextureUsage::COLOR_ATTACHMENT);

                    normal = builder.readTexture(normal);
                    albedo = builder.readTexture(albedo);
                    depth = builder.readTexture(depth);

                    RenderGraphPassDescriptor desc{.textures = {output, albedo, depth, normal}};
                    desc.addSubpass({
                                            .inputAttachments = {
                                                    albedo, depth,
                                                    normal
                                            },
                                            .outputAttachments = {output}
                                    });
                    builder.declare("Lighting Pass", desc);
                    // builder.addSubPass();
                },
                [&](RenderPassContext & context) {
                    renderContext->getPipelineState().setPipelineLayout(*pipelineLayouts.lighting).setRasterizationState({.cullMode = VK_CULL_MODE_NONE}).setDepthStencilState({.depthWriteEnable =  false});
                    struct {
                        glm::mat4 invView{};
                        glm::vec2 invRes{};
                    } fragUniform;
                    
                    fragUniform.invRes = glm::vec2(1.0f / renderContext->getSwapChainExtent().width,
                                                   1.0f / renderContext->getSwapChainExtent().height);
                    fragUniform.invView = glm::inverse(camera->matrices.perspective * camera->matrices.view);

                    auto viewBuffer = renderContext->
                            allocateBuffer(sizeof(fragUniform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
                    viewBuffer.buffer->uploadData(&fragUniform, viewBuffer.size, viewBuffer.offset);

                    renderContext->bindBuffer(3, *viewBuffer.buffer, viewBuffer.offset, viewBuffer.size)
                                .bindInput(0, blackBoard.getImageView("albedo"), 0, 0)
                                .bindInput(0, blackBoard.getImageView("depth"), 1, 0)
                                .bindInput(0,blackBoard.getImageView("normal"),2,0)
                                .bindLight<DeferredLights>(scene->getLights(), 0, 4)
                                .flushAndDraw(commandBuffer,3,1,0,0);
                    
                });
    }


    gui->addGuiPass(rg, *renderContext);


    rg.execute(commandBuffer);
    renderContext->submitAndPresent(commandBuffer, fence);
}


void Example::prepare() {
    Application::prepare();

    //  scene = GltfLoading::LoadSceneFromGLTFFile(*device, FileUtils::getResourcePath("space_module/SpaceModule.gltf"));
    scene = GltfLoading::LoadSceneFromGLTFFile(*device, FileUtils::getResourcePath("sponza/Sponza01.gltf"));

    auto light_pos = glm::vec3(0.0f, 128.0f, -225.0f);
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
                props.color = light_color;
                props.intensity = 0.2f;
                props.position = pos;

                scene->addLight(Light{.type = LIGHT_TYPE::Point, .lightProperties = props});
            }
        }
    }
    std::vector<Shader> shaders{
            Shader(*device, FileUtils::getShaderPath("defered.vert")),
            Shader(*device, FileUtils::getShaderPath("defered.frag"))
    };
    pipelineLayouts.gBuffer = std::make_unique<PipelineLayout>(*device, shaders);
    std::vector<Shader> shaders1{
            Shader(*device, FileUtils::getShaderPath("lighting.vert")),
            Shader(*device, FileUtils::getShaderPath("lighting.frag"))
    };
    pipelineLayouts.lighting = std::make_unique<PipelineLayout>(*device, shaders1);

        camera->flipY = true;
        camera->setTranslation(glm::vec3(0.0f, 1.0f, 0.0f));
        camera->setTranslation(glm::vec3(-705.f, 200.f, -119.f));
        camera->setRotation(glm::vec3(0.0f, -90.0f, 0.0f));
        camera->setRotation(glm::vec3(0.0f, -90.0f, 0.0f));
        camera->setPerspective(60.0f, (float) width / (float) height, 0.1f, 4000.f);
}


Example::Example() : Application("Drawing Triangle", 1024, 1024) {
        addDeviceExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
}


void Example::onUpdateGUI() {
    gui->text("Hello");
    gui->text("Hello IMGUI");
    gui->text("Hello imgui");
    gui->checkBox("Use subpasses", &useSubpass);
    // ImGui::RadioButton("use subpass", &useSubpass, 1);
}

int main() {
    auto example = new Example();
    example->prepare();
    example->mainloop();
    return 0;
}
