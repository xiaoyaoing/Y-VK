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
    struct GBufferPassData
    {
        RenderGraphId<RenderGraphTexture> position;


        RenderGraphId<RenderGraphTexture> albedo;

        RenderGraphId<RenderGraphTexture> normal;

        RenderGraphId<RenderGraphTexture> depth;


        RenderGraphId<RenderGraphTexture> output;
    };

    vkWaitForFences(device->getHandle(), 1, &fence, VK_TRUE, UINT64_MAX);
    vkResetFences(device->getHandle(), 1, &fence);
    graph.addPass<GBufferPassData>("gbuffer", [&](RenderGraph::Builder& builder, GBufferPassData& data)
                                   {
                                       //                                       data.output = graph.create<RenderGraphTexture>("output",
                                       //                                           {
                                       //                                               .extent = renderContext->getSwapChainExtent(),
                                       //                                               .format = VK_FORMAT_R8G8B8A8_SRGB,
                                       //                                               .usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                       //                                               .memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY
                                       //                                           });
                                       data.albedo = graph.create<RenderGraphTexture>("color",
                                           {
                                               .extent = renderContext->getSwapChainExtent(),
                                               .format = VK_FORMAT_R8G8B8A8_SRGB,
                                               .usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                               VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
                                               .memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY
                                           });

                                       data.normal = graph.create<RenderGraphTexture>("normal",
                                           {
                                               .extent = renderContext->getSwapChainExtent(),
                                               .format = VK_FORMAT_R8G8B8A8_SRGB,
                                               .usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                               VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
                                               .memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY
                                           });

                                       data.depth = graph.create<RenderGraphTexture>("depth", {
                                           .extent = renderContext->getSwapChainExtent(),
                                           .format = VK_FORMAT_D32_SFLOAT,
                                           .usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                                           VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
                                           .memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY
                                       });

                                       data.position = graph.create<RenderGraphTexture>("position", {
                                           .extent = renderContext->getSwapChainExtent(),
                                           .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                                           .usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                           VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
                                           .memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY
                                       });

                                       data.output = graph.import("output", &renderContext->getCurHwtexture());


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
                                                       }
                                       );
                                   },
                                   [&](GBufferPassData& data, const RenderPassContext& context)
                                   {
                                       auto& commandBuffer = context.commandBuffer;

                                       //commandBuffer.bindPipeline(context.pipeline);
                                       renderContext->getPipelineState().setSubpassIndex(0);

                                       renderContext->getPipelineState().setPipelineLayout(*pipelineLayouts.gBuffer);
                                       renderContext->getPipelineState().setRenderPass(context.renderPass);
                                       // renderContext->clearResouceSets();

                                       renderContext->clearResourceSets();

                                       //  sponza->iterateAllNodes([&](CommandBuffer  &))

                                       renderContext->draw(commandBuffer, *sponza);


                                       // Update blend state attachments
                                       // auto blend_state = pipeline_state.get_color_blend_state();
                                       // blend_state.attachments.resize(current_render_pass.render_pass->get_color_output_count(pipeline_state.get_subpass_index()));
                                       // pipeline_state.set_color_blend_state(blend_state);

                                       // // Reset descriptor sets
                                       // resource_binding_state.reset();
                                       // descriptor_set_layout_binding_state.clear();
                                       //
                                       // // Clear stored push constants
                                       // stored_push_constants.clear();

                                       vkCmdNextSubpass(commandBuffer.getHandle(), VK_SUBPASS_CONTENTS_INLINE);


                                       renderContext->getPipelineState().setPipelineLayout(*pipelineLayouts.lighting);
                                       renderContext->getPipelineState().setRasterizationState({
                                           .cullMode = VK_CULL_MODE_NONE
                                       });
                                       renderContext->getPipelineState().setSubpassIndex(1);
                                       renderContext->clearResourceSets();
                                       ColorBlendState colorBlendState = renderContext->getPipelineState().
                                           getColorBlendState();
                                       colorBlendState.attachments.resize(context.renderPass.getColorOutputCount(1));
                                       renderContext->getPipelineState().setColorBlendState(colorBlendState);
                                       renderContext->bindInput(
                                           0, context.renderTarget.getHwTextures()[2]->getVkImageView(), 0, 0);
                                       renderContext->bindInput(
                                           0, context.renderTarget.getHwTextures()[3]->getVkImageView(), 1, 0);
                                       renderContext->bindInput(
                                           0, context.renderTarget.getHwTextures()[4]->getVkImageView(), 2, 0);
                                       renderContext->drawLightingPass(commandBuffer);

                                       //    commandBuffer.draw(3,1,0,0);

                                       commandBuffer.endRenderPass();
                                   });

    // struct LightingPassData {
    //
    //     RenderGraphId<RenderGraphTexture> position;
    //
    //     RenderGraphId<RenderGraphTexture> normal;
    //     
    //     RenderGraphId<RenderGraphTexture> albedo;
    //
    //     RenderGraphId<RenderGraphTexture> output;
    // };
    // graph.addPass<LightingPassData>("lighting", [&](RenderGraph::Builder &builder, LightingPassData &data) {
    //                                   data.albedo = graph.getBlackBoard().get<RenderGraphTexture>("albedo");
    //                                   data.position = graph.getBlackBoard().get<RenderGraphTexture>("position");
    //                                   data.normal = graph.getBlackBoard().get<RenderGraphTexture>("normal");
    //
    //                                   builder.declare("lightingPass",{.color = {data.albedo,data.position,data.normal,data.output}});
    //                                     
    //     
    //                                },
    //                                 [&](LightingPassData &data, const RenderPassContext &context) {
    //                                     
    //                                     renderContext->getPipelineState().setPipelineLayout(*pipelineLayouts.lighting);
    //
    //                                     context.commandBuffer.draw(3,1,0,0);
    //                                     
    //                                 });

    //  graph.addPresentPass(graph.getBlackBoard().get<RenderGraphTexture>("outPut"));
    struct ImageCopy
    {
        RenderGraphId<RenderGraphTexture> src;
        RenderGraphId<RenderGraphTexture> dst;
    };
    // graph.addPass<ImageCopy>("copy", [&](RenderGraph::Builder& builder, ImageCopy& data)
    //                          {
    //                              data.src = graph.getBlackBoard().get<RenderGraphTexture>("output");
    //                              data.dst = graph.import("dst", &renderContext->getCurHwtexture());
    //                          },
    //                          [&](ImageCopy& data, const RenderPassContext& context)
    //                          {
    //                              auto& commandBuffer = context.commandBuffer;
    //                              commandBuffer.copyImage(data.src, data.dst);
    //                              commandBuffer.endRecord();
    //                          });


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
