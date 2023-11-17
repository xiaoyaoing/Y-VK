//
// Created by pc on 2023/8/17.
//

#include "RenderGraphTest.h"
#include "Shader.h"
#include "../../framework/Common/VkCommon.h"
#include "FIleUtils.h"
#include "Scene/SceneUtil.h"

void Example::drawFrame()
{
    renderContext->camera = camera.get();
    auto& commandBuffer = renderContext->beginFrame();


    RenderGraph graph(*device);
    auto& blackBoard = graph.getBlackBoard();

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


                blackBoard.put("albedo", data.albedo);
                blackBoard.put("normal", data.normal);
                blackBoard.put("depth", data.depth);
                blackBoard.put("position", data.position);
                blackBoard.put("output", data.output);

                RenderGraphPassDescriptor desc{
                    .textures = {
                        data.output, data.depth, data.albedo, data.position,
                        data.normal
                    }
                };
                desc.addSubpass({.outputAttachments = {data.albedo, data.position, data.normal, data.depth}});
                desc.addSubpass({
                    .inputAttachments = {data.albedo, data.position, data.normal}, .outputAttachments = {data.output}
                });

                builder.declare("Color Pass Target", desc);

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
                renderContext->getPipelineState().setPipelineLayout(*pipelineLayouts.gBuffer);

                sponza->bindBuffer(commandBuffer);
                sponza->IteratePrimitives([&](gltfLoading::Primitive& primitive)
                    {
                        VertexInputState vertexInputState{};
                        vertexInputState.bindings = {Vertex::getBindingDescription()};
                        vertexInputState.attributes = Vertex::getAttributeDescriptions();
                        //renderContext->getPipelineState().setVertexInputState(vertexInputState);

                        //vertexInputState = SceneUtil::getPrimitiveVertexInputState(primitive, renderContext->getPipelineState().getPipelineLayout());
                        //   renderContext->getPipelineState().setVertexInputState(vertexInputState);
                        renderContext->bindPrimitive(primitive);


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
                            commandBuffer, primitive.indexCount, 1, 0, 0, 0);
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

                renderContext->bindInput(0, blackBoard.getImageView("albedo"), 0, 0);
                renderContext->bindInput(0, blackBoard.getImageView("normal"), 1, 0);
                renderContext->bindInput(0, blackBoard.getImageView("position"), 2, 0);


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

                RenderGraphPassDescriptor desc{.textures = {data.depth, data.albedo, data.position, data.normal}};
                builder.declare("GBuffer", desc);


                data.normal = builder.writeTexture(data.normal, TextureUsage::COLOR_ATTACHMENT);
                data.albedo = builder.writeTexture(data.albedo, TextureUsage::COLOR_ATTACHMENT);
                data.position = builder.writeTexture(data.position, TextureUsage::COLOR_ATTACHMENT);
                data.depth = builder.writeTexture(data.depth, TextureUsage::DEPTH_ATTACHMENT);
                data.depth = builder.readTexture(data.depth, TextureUsage::DEPTH_ATTACHMENT);


                blackBoard.put("albedo", data.albedo);
                blackBoard.put("normal", data.normal);
                blackBoard.put("position", data.position);
            },
            [&](GBufferData& data, const RenderPassContext& context)
            {
                //   renderContext->beginRenderPass(commandBuffer, context.renderTarget, {});
                renderContext->getPipelineState().setPipelineLayout(*pipelineLayouts.gBuffer);
                sponza->bindBuffer(commandBuffer);
                sponza->IteratePrimitives([&](gltfLoading::Primitive& primitive)
                    {
                        VertexInputState vertexInputState{};
                        vertexInputState.bindings = {Vertex::getBindingDescription()};
                        vertexInputState.attributes = Vertex::getAttributeDescriptions();
                        renderContext->getPipelineState().setVertexInputState(vertexInputState);

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
                data.position = blackBoard["position"];
                data.normal = blackBoard["normal"];
                data.albedo = blackBoard["albedo"];
                data.output = graph.importTexture("output", &renderContext->getCurHwtexture());

                data.output = builder.writeTexture(data.output, TextureUsage::COLOR_ATTACHMENT);
                data.output = builder.readTexture(data.output, TextureUsage::COLOR_ATTACHMENT);

                data.normal = builder.readTexture(data.normal, {});
                data.albedo = builder.readTexture(data.albedo, {});
                data.position = builder.readTexture(data.position, {});

                RenderGraphPassDescriptor desc{.textures = {data.output, data.albedo, data.position, data.normal}};
                desc.addSubpass({
                    .inputAttachments = {data.albedo, data.position, data.normal}, .outputAttachments = {data.output}
                });
                builder.declare("Lighting Pass", desc);
                // builder.addSubPass();
            },
            [&](LightingData& data, const RenderPassContext& context)
            {
                // SubpassInfo lightingSubpass = {.inputAttachments = {1, 2, 3}, .outputAttachments = {0}};

                // renderContext->beginRenderPass(commandBuffer, context.renderTarget, {lightingSubpass});

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


                renderContext->bindInput(0, blackBoard.getImageView("albedo"), 0, 0);
                renderContext->bindInput(0, blackBoard.getImageView("normal"), 1, 0);
                renderContext->bindInput(0, blackBoard.getImageView("position"), 2, 0);


                renderContext->getPipelineState().setRasterizationState({
                    .cullMode = VK_CULL_MODE_NONE
                });
                renderContext->flushAndDraw(commandBuffer, 3, 1, 0, 0);
                renderContext->endRenderPass(commandBuffer);
            });
    }


    graph.execute(commandBuffer);
    renderContext->submit(commandBuffer, fence);
}


void Example::prepare()
{
    Application::prepare();

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


Example::Example() : Application("Drawing Triangle", 1024, 1024)
{
    camera = std::make_unique<Camera>();
    camera->flipY = true;
    camera->setTranslation(glm::vec3(0.0f, 1.0f, 0.0f));
    camera->setRotation(glm::vec3(0.0f, -90.0f, 0.0f));
    camera->setPerspective(60.0f, (float)width / (float)height, 0.1f, 256.0f);
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
