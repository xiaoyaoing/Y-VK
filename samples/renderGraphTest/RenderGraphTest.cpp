//
// Created by pc on 2023/8/17.
//

#include "RenderGraphTest.h"
#include "Shader.h"
#include "../../framework/Common/VkCommon.h"
#include "FIleUtils.h"

void Example::drawFrame()
{
    renderContext->camera = camera.get();
    // auto& graph = renderContext->getRenderGraph();

    // renderContext->getPipelineState().setViewportState(ViewportState{.})
    auto& commandBuffer = renderContext->beginFrame();


    // commandBufffer
    RenderGraph graph(*device);


    vkWaitForFences(device->getHandle(), 1, &fence, VK_TRUE, UINT64_MAX);
    vkResetFences(device->getHandle(), 1, &fence);

    //ON PASS TWO SUBPASS
    struct OnePassTwoSubPassDeferedShadingData
    {
        RenderGraphHandle position;


        RenderGraphHandle albedo;

        RenderGraphHandle normal;

        RenderGraphHandle depth;


        RenderGraphHandle output;
    };

    bool useSubpass = false;

    if (useSubpass)
    {
        graph.addPass<OnePassTwoSubPassDeferedShadingData>(
            "gbuffer", [&](RenderGraph::Builder& builder, OnePassTwoSubPassDeferedShadingData& data)
            {
                data.albedo = graph.createTexture("color",
                                                  {
                                                      .extent = renderContext->getSwapChainExtent(),
                                                      .useage = TextureUsage::SUBPASS_INPUT |
                                                      TextureUsage::COLOR_ATTACHMENT
                                                  });

                data.normal = graph.createTexture("normal",
                                                  {
                                                      .extent = renderContext->getSwapChainExtent(),
                                                      .useage = TextureUsage::SUBPASS_INPUT |
                                                      TextureUsage::COLOR_ATTACHMENT

                                                  });

                data.depth = graph.createTexture("depth", {
                                                     .extent = renderContext->getSwapChainExtent(),
                                                     .useage = TextureUsage::SUBPASS_INPUT |
                                                     TextureUsage::DEPTH_ATTACHMENT

                                                 });

                data.position = graph.createTexture("position", {
                                                        .extent = renderContext->getSwapChainExtent(),
                                                        .useage = TextureUsage::SUBPASS_INPUT |
                                                        TextureUsage::COLOR_ATTACHMENT
                                                    });

                data.output = graph.importTexture("output", &renderContext->getCurHwtexture());


                graph.getBlackBoard().put("albedo", data.albedo);
                graph.getBlackBoard().put("normal", data.normal);
                graph.getBlackBoard().put("depth", data.depth);
                graph.getBlackBoard().put("position", data.position);
                graph.getBlackBoard().put("output", data.output);

                builder.declare("Color Pass Target", {
                                    .color = {
                                        data.output, data.depth, data.albedo, data.position,
                                        data.normal
                                    }
                                });

                data.output = builder.writeTexture(data.output, TextureUsage::COLOR_ATTACHMENT);
                data.position = builder.writeTexture(data.position, TextureUsage::COLOR_ATTACHMENT);
                data.normal = builder.writeTexture(
                    data.normal, TextureUsage::COLOR_ATTACHMENT);
                data.albedo = builder.writeTexture(
                    data.albedo, TextureUsage::COLOR_ATTACHMENT);
                data.depth = builder.writeTexture(data.depth, TextureUsage::DEPTH_ATTACHMENT);

                data.output = builder.readTexture(data.output, TextureUsage::COLOR_ATTACHMENT);
                data.position = builder.readTexture(
                    data.position, TextureUsage::COLOR_ATTACHMENT);
                data.normal = builder.readTexture(
                    data.normal, TextureUsage::COLOR_ATTACHMENT);
                data.albedo = builder.readTexture(
                    data.albedo, TextureUsage::COLOR_ATTACHMENT);
                data.depth = builder.readTexture(data.depth, TextureUsage::DEPTH_ATTACHMENT);
            },
            [&](OnePassTwoSubPassDeferedShadingData& data, const RenderPassContext& context)
            {
                //  context.renderTarget
                std::vector<SubpassInfo> subpassInfos{};
                SubpassInfo gBufferSubPassInfo = {
                    .inputAttachments = {}, .outputAttachments = {2, 3, 4}
                };
                SubpassInfo LightingSubPassInfo = {
                    .inputAttachments = {2, 3, 4}, .outputAttachments = {0}
                };


                renderContext->beginRenderPass(commandBuffer, context.renderTarget,
                                               {gBufferSubPassInfo, LightingSubPassInfo});
                renderContext->getPipelineState().setPipelineLayout(*pipelineLayouts.gBuffer);

                sponza->bindBuffer(commandBuffer);
                sponza->IteratePrimitives([&](gltfLoading::Primitive& primitive)
                    {
                        VertexInputState vertexInputState{};
                        vertexInputState.bindings = {Vertex::getBindingDescription()};
                        vertexInputState.attributes = Vertex::getAttributeDescriptions();
                        renderContext->getPipelineState().setVertexInputState(vertexInputState);
                        // renderContext->getPipelineState().setVertexInputState(vertexInputState);

                        const auto allocation = renderContext->allocateBuffer(
                            sizeof(GlobalUniform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

                        //todo: use camera data here
                        GlobalUniform uniform{
                            .model = primitive.matrix, .view = camera->matrices.view,
                            .proj = camera->matrices.perspective
                        };
                        allocation.buffer->uploadData(
                            &uniform, allocation.size, allocation.offset);

                        renderContext->bindBuffer(
                            0, *allocation.buffer, allocation.offset, allocation.size, 0,
                            0);
                        renderContext->bindMaterial(primitive.material);
                        renderContext->flushAndDrawIndexed(
                            commandBuffer, primitive.indexCount, 1, primitive.firstIndex, 0, 0);
                    }
                );
                renderContext->nextSubpass(commandBuffer);
                renderContext->getPipelineState().setPipelineLayout(*pipelineLayouts.lighting);

                struct Poses
                {
                    glm::vec3 cameraPos, lightPos;
                };

                auto buffer = renderContext->allocateBuffer(
                    sizeof(Poses), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
                const Poses poses{.cameraPos = camera->position, .lightPos = {0.0f, 2.5f, 0.0f}};
                buffer.buffer->uploadData(&poses, buffer.size, buffer.offset);
                renderContext->bindBuffer(0, *buffer.buffer, buffer.offset, buffer.size, 3, 0);

                renderContext->bindInput(
                    0, context.renderTarget.getHwTextures()[2]->getVkImageView(), 0, 0);
                renderContext->bindInput(
                    0, context.renderTarget.getHwTextures()[3]->getVkImageView(), 1, 0);
                renderContext->bindInput(
                    0, context.renderTarget.getHwTextures()[4]->getVkImageView(), 2, 0);


                renderContext->getPipelineState().setRasterizationState({
                    .cullMode = VK_CULL_MODE_NONE
                });

                renderContext->flushAndDraw(commandBuffer, 3, 1, 0, 0);

                renderContext->endRenderPass(commandBuffer);
            });
    }


    //Two RenderPass
    else
    {
        struct GBufferData
        {
            RenderGraphHandle position;
            RenderGraphHandle albedo;
            RenderGraphHandle normal;
            RenderGraphHandle depth;
        };
        graph.addPass<GBufferData>(
            "GBufferPass", [&](RenderGraph::Builder& builder, GBufferData& data)
            {
                data.albedo = graph.createTexture("albedo",
                                                  {
                                                      .extent = renderContext->getSwapChainExtent(),
                                                      .useage = TextureUsage::SUBPASS_INPUT |
                                                      TextureUsage::COLOR_ATTACHMENT
                                                  });

                data.normal = graph.createTexture("normal",
                                                  {
                                                      .extent = renderContext->getSwapChainExtent(),
                                                      .useage = TextureUsage::SUBPASS_INPUT |
                                                      TextureUsage::COLOR_ATTACHMENT

                                                  });

                data.position = graph.createTexture("position", {
                                                        .extent = renderContext->getSwapChainExtent(),
                                                        .useage = TextureUsage::SUBPASS_INPUT |
                                                        TextureUsage::COLOR_ATTACHMENT
                                                    });

                data.depth = graph.createTexture("depth", {
                                                     .extent = renderContext->getSwapChainExtent(),
                                                     .useage = TextureUsage::SUBPASS_INPUT |
                                                     TextureUsage::DEPTH_ATTACHMENT

                                                 });

                builder.declare("GBuffer Pass", {.color = {data.depth,data.albedo, data.position, data.normal}});


                data.normal = builder.writeTexture(data.normal, TextureUsage::COLOR_ATTACHMENT);
                data.albedo = builder.writeTexture(data.albedo, TextureUsage::COLOR_ATTACHMENT);
                data.position = builder.writeTexture(data.position, TextureUsage::COLOR_ATTACHMENT);
                data.depth = builder.writeTexture(data.depth, TextureUsage::DEPTH_ATTACHMENT);
                data.depth = builder.readTexture(data.depth, TextureUsage::DEPTH_ATTACHMENT);


                graph.getBlackBoard().put("albedo", data.albedo);
                graph.getBlackBoard().put("normal", data.normal);
                graph.getBlackBoard().put("position", data.position);
            },
            [&](GBufferData& data, const RenderPassContext& context)
            {
                renderContext->beginRenderPass(commandBuffer, context.renderTarget, {});
                renderContext->getPipelineState().setPipelineLayout(*pipelineLayouts.gBuffer);
                sponza->bindBuffer(commandBuffer);
                sponza->IteratePrimitives([&](gltfLoading::Primitive& primitive)
                    {
                        VertexInputState vertexInputState{};
                        vertexInputState.bindings = {Vertex::getBindingDescription()};
                        vertexInputState.attributes = Vertex::getAttributeDescriptions();
                        renderContext->getPipelineState().setVertexInputState(vertexInputState);
                        // renderContext->getPipelineState().setVertexInputState(vertexInputState);

                        const auto allocation = renderContext->allocateBuffer(
                            sizeof(GlobalUniform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

                        //todo: use camera data here
                        GlobalUniform uniform{
                            .model = primitive.matrix, .view = camera->matrices.view,
                            .proj = camera->matrices.perspective
                        };
                        allocation.buffer->uploadData(
                            &uniform, allocation.size, allocation.offset);

                        renderContext->bindBuffer(
                            0, *allocation.buffer, allocation.offset, allocation.size, 0,
                            0);
                        renderContext->bindMaterial(primitive.material);
                        renderContext->flushAndDrawIndexed(
                            commandBuffer, primitive.indexCount, 1, primitive.firstIndex, 0, 0);
                    }
                );
                renderContext->endRenderPass(commandBuffer);
            });

        struct LightingData
        {
            RenderGraphHandle position;
            RenderGraphHandle albedo;
            RenderGraphHandle normal;
            RenderGraphHandle output;
        };

        graph.addPass<LightingData>(
            "LightingPass", [&](RenderGraph::Builder& builder, LightingData& data)
            {
                data.position = graph.getBlackBoard()["position"];
                data.normal = graph.getBlackBoard()["normal"];
                data.albedo = graph.getBlackBoard()["albedo"];
                data.output = graph.importTexture("output", &renderContext->getCurHwtexture());

                data.output = builder.writeTexture(data.output, TextureUsage::COLOR_ATTACHMENT);
                data.output = builder.readTexture(data.output, TextureUsage::COLOR_ATTACHMENT);

                data.normal = builder.readTexture(data.normal, {});
                data.albedo = builder.readTexture(data.albedo, {});
                data.position = builder.readTexture(data.position, {});

                builder.declare("Lighting Pass", {.color = {data.output, data.albedo, data.position, data.normal}});
            },
            [&](LightingData& data, const RenderPassContext& context)
            {
                auto hwTextures = context.renderTarget.getHwTextures();
                hwTextures[1]->getVkImage().transitionLayout(commandBuffer, VulkanLayout::READ_ONLY,
                                                             hwTextures[1]->getVkImageView().getSubResourceRange());
                hwTextures[2]->getVkImage().transitionLayout(commandBuffer, VulkanLayout::READ_ONLY,
                                                             hwTextures[2]->getVkImageView().getSubResourceRange());
                hwTextures[3]->getVkImage().transitionLayout(commandBuffer, VulkanLayout::READ_ONLY,
                                                             hwTextures[3]->getVkImageView().getSubResourceRange());
                 //  context.renderTarget.getHwTextures()[2]->getVkImage().transitionLayout(commandBuffer,{});
                 // context.renderTarget.getHwTextures()[3]->getVkImage().transitionLayout(commandBuffer,{});

                SubpassInfo lightingSubpass = {.inputAttachments = {1, 2, 3}, .outputAttachments = {0}};
                renderContext->beginRenderPass(commandBuffer, context.renderTarget, {lightingSubpass});
                renderContext->getPipelineState().setPipelineLayout(*pipelineLayouts.lighting);

                struct Poses
                {
                    glm::vec3 cameraPos, lightPos;
                };

                auto buffer = renderContext->allocateBuffer(
                    sizeof(Poses), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
                const Poses poses{.cameraPos = camera->position, .lightPos = {0.0f, 2.5f, 0.0f}};
                buffer.buffer->uploadData(&poses, buffer.size, buffer.offset);
                renderContext->bindBuffer(0, *buffer.buffer, buffer.offset, buffer.size, 3, 0);

                renderContext->bindInput(
                    0, context.renderTarget.getHwTextures()[1]->getVkImageView(), 0, 0);
                renderContext->bindInput(
                    0, context.renderTarget.getHwTextures()[2]->getVkImageView(), 1, 0);
                renderContext->bindInput(
                    0, context.renderTarget.getHwTextures()[3]->getVkImageView(), 2, 0);


                renderContext->getPipelineState().setRasterizationState({
                    .cullMode = VK_CULL_MODE_NONE
                });
                renderContext->flushAndDraw(commandBuffer, 3, 1, 0, 0);
                renderContext->endRenderPass(commandBuffer);
            });
    }


    graph.execute(commandBuffer);
    //  commandBuffer->endRecord();
    renderContext->submit(commandBuffer, fence);
}

void Example::prepareUniformBuffers()
{
    uniform_buffers.scene = std::make_unique<Buffer>(*device, sizeof(ubo_vs), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                     VMA_MEMORY_USAGE_CPU_TO_GPU);
    uniform_buffers.scene->uploadData(&ubo_vs, sizeof(ubo_vs));

    // textures.texture1 = Texture::loadTexture(*device, FileUtils::getResourcePath() + "Window.png");
}

void Example::createDescriptorSet()
{
    // descriptorSet = std::make_unique<DescriptorSet>(*device, *descriptorPool, *descriptorLayout, 1);
    // descriptorSet->updateBuffer({uniform_buffers.scene.get()}, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    //
    // //textures.texture1 = Texture::loadTexture(*device, FileUtils::getResourcePath() + "Window.png");
    //
    // auto imageDescriptorInfo = vkCommon::initializers::descriptorImageInfo(textures.texture1);
    //
    // descriptorSet->updateImage({imageDescriptorInfo}, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
}

void Example::createDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> poolSizes = {
        vkCommon::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2),
        vkCommon::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2)
    };

    descriptorPool = std::make_unique<DescriptorPool>(*device, poolSizes, 2);
}

void Example::updateUniformBuffers()
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    ubo_vs.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo_vs.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo_vs.proj = glm::perspective(glm::radians(45.0f), 1.f, 0.1f, 10.0f);
    ubo_vs.proj[1][1] *= -1;

    ubo_vs.view = camera->matrices.view;
    ubo_vs.proj = camera->matrices.perspective;
    // ubo_vs.model = glm::mat4::Ide
    uniform_buffers.scene->uploadData(&ubo_vs, sizeof(ubo_vs));
}

void Example::createGraphicsPipeline()
{
    // todo handle shader complie
}

void Example::prepare()
{
    Application::prepare();

    prepareUniformBuffers();
    //  createDescriptorSetLayout();
    //   createDescriptorPool();
    //   createDescriptorSet();

    createGraphicsPipeline();

    buildCommandBuffers();

    sponza = gltfLoading::Model::loadFromFile(*device, FileUtils::getResourcePath("sponza/sponza.gltf"));

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
}

void Example::createDescriptorSetLayout()
{
    descriptorLayout = std::make_unique<DescriptorLayout>(*device);
    descriptorLayout->addBinding(VK_SHADER_STAGE_VERTEX_BIT, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0);
    descriptorLayout->addBinding(VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0);
    descriptorLayout->createLayout(0);
}

Example::Example() : Application("Drawing Triangle", 1024, 1024)
{
    camera = std::make_unique<Camera>();
    camera->flipY = true;
    camera->setTranslation(glm::vec3(0.0f, 1.0f, 0.0f));
    camera->setRotation(glm::vec3(0.0f, -90.0f, 0.0f));
    camera->setPerspective(60.0f, (float)width / (float)height, 0.1f, 256.0f);
}

void Example::bindUniformBuffers(CommandBuffer& commandBuffer)
{
    commandBuffer.bindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayOut, 0, {*descriptorSet.get()},
                                     {});
}

void Example::buildCommandBuffers()
{
    // for (int i = commandBuffers.size() - 1; i >= 0; i--)
    // {
    //     auto& commandBuffer = *commandBuffers[i];
    //     commandBuffer.beginRecord(0);
    //     commandBuffer.bindPipeline(graphicsPipeline->getHandle());
    //     renderContext->setActiveFrameIdx(i);
    //     bindUniformBuffers(commandBuffer);
    //     draw(commandBuffer);
    //     commandBuffer.endRecord();
    // }
}

void Example::draw(CommandBuffer& commandBuffer)
{
    bindUniformBuffers(commandBuffer);
    renderPipeline->draw(commandBuffer);
    commandBuffer.beginRenderPass(renderPipeline->getRenderPass(),
                                  Default::clearValues(), VkSubpassContents{});
    const VkViewport viewport = vkCommon::initializers::viewport((float)width, (float)height, 0.0f, 1.0f);
    const VkRect2D scissor = vkCommon::initializers::rect2D(width, height, 0, 0);
    vkCmdSetViewport(commandBuffer.getHandle(), 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer.getHandle(), 0, 1, &scissor);
    gui->draw(commandBuffer.getHandle());

    commandBuffer.endRenderPass();
}

void Example::onUpdateGUI()
{
    gui->text("Hello");
    gui->text("Hello IMGUI");
    gui->text("Hello imgui");
}

int main()
{
    auto example = new Example();
    example->prepare();
    example->mainloop();
    return 0;
}

// Example *example{nullptr};
// LRESULT __stdcall WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
// {
//     if (example)
//     {
//         example->handleMessages(hWnd, uMsg, wParam, lParam);
//     }
//     return (DefWindowProcA(hWnd, uMsg, wParam, lParam));
// }
// int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
// {
//     example = new Example();
//     example->prepare();
//     example->mainloop();
//     delete example();
//     return 0;
// }
