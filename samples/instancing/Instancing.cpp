//
// Created by pc on 2023/8/17.
//

#include <random>
#include "Instancing.h"
#include "Core/Shader/Shader.h"
#include "../../framework/Common/VkCommon.h"
#include "Common/FIleUtils.h"

#define VERTEX_BUFFER_BIND_ID 0
#define INSTANCE_BUFFER_BIND_ID 1
#define INSTANCE_NUM 8192
#define M_PI 3.14159265358979323846

void Example::prepareUniformBuffers() {
    uniform_buffers.scene = std::make_unique<Buffer>(*device, sizeof(ubo_vs), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                     VMA_MEMORY_USAGE_CPU_TO_GPU);
    uniform_buffers.scene->uploadData(&ubo_vs, sizeof(ubo_vs));
}

void Example::loadResources() {
    textures.rocketTexture = Texture::loadTextureArray(*device, FileUtils::getResourcePath(
            "textures/texturearray_rocks_rgba.ktx"));
    textures.planetTexture = Texture::loadTexture(*device,
                                                  FileUtils::getResourcePath("textures/lavaplanet_rgba.ktx"));
    const uint32_t glTFLoadingFlags = gltfLoading::FileLoadingFlags::PreTransformVertices |
                                      gltfLoading::FileLoadingFlags::PreMultiplyVertexColors |
                                      gltfLoading::FileLoadingFlags::FlipY;

    models.planets = std::make_unique<gltfLoading::GLTFLoadingImpl>(*device);
    models.planets->loadFromFile(FileUtils::getResourcePath("models/lavaplanet.gltf"), glTFLoadingFlags);

    models.rockets = std::make_unique<gltfLoading::GLTFLoadingImpl>(*device);
    models.rockets->loadFromFile(FileUtils::getResourcePath("models/rock01.gltf"), glTFLoadingFlags);
}

void Example::createDescriptorSet() {
    descriptors.planetDescriptor = std::make_unique<DescriptorSet>(*device, *descriptorPool, *descriptorLayout, 1);
    descriptors.planetDescriptor->updateBuffer({uniform_buffers.scene.get()}, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    descriptors.planetDescriptor->updateImage({vkCommon::initializers::descriptorImageInfo(textures.planetTexture)}, 1,
                                              VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

    descriptors.rockDescriptor = std::make_unique<DescriptorSet>(*device, *descriptorPool, *descriptorLayout, 1);
    descriptors.rockDescriptor->updateBuffer({uniform_buffers.scene.get()}, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    descriptors.rockDescriptor->updateImage({vkCommon::initializers::descriptorImageInfo(textures.rocketTexture)}, 1,
                                            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
}

void Example::createDescriptorPool() {
    std::vector<VkDescriptorPoolSize> poolSizes = {
            vkCommon::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2),
            vkCommon::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2)
    };

    descriptorPool = std::make_unique<DescriptorPool>(*device, poolSizes, 2);
}

void Example::updateUniformBuffers() {
    // if ()
    {
        ubo_vs.projection = camera->matrices.perspective;
        ubo_vs.view = camera->matrices.view;
    }

    //  if (!paused)
    {
        ubo_vs.locSpeed += frameTimer * 0.35f;
        ubo_vs.globSpeed += frameTimer * 0.01f;
    }
    // ubo_vs.model = glm::mat4::Ide
    uniform_buffers.scene->uploadData(&ubo_vs, sizeof(ubo_vs));
}

void Example::createGraphicsPipeline() {
    // todo handle shader complie

    VkPipelineShaderStageCreateInfo shaderStages[2];

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

    bindingDescriptions = {
            vkCommon::initializers::vertexInputBindingDescription(VERTEX_BUFFER_BIND_ID,
                                                                  sizeof(gltfLoading::Vertex),
                                                                  VK_VERTEX_INPUT_RATE_VERTEX),
            // 每个顶点属性的数据在一组顶点之间跨越。每个实例之间使用相同的顶点属性数据。
            vkCommon::initializers::vertexInputBindingDescription(INSTANCE_BUFFER_BIND_ID,
                                                                  sizeof(InstanceData),
                                                                  VK_VERTEX_INPUT_RATE_INSTANCE)
    };

    // Vertex attribute bindings
    // Note that the shader declaration for per-vertex and per-instance attributes is the same, the different input rates are only stored in the bindings:
    // instanced.vert:
    //	layout (location = 0) in vec3 inPos;		Per-Vertex
    //	...
    //	layout (location = 4) in vec3 instancePos;	Per-Instance
    attributeDescriptions = {
            // Per-vertex attributes
            // These are advanced for each vertex fetched by the vertex shader
            vkCommon::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 0,
                                                                    VK_FORMAT_R32G32B32_SFLOAT,
                                                                    0), // Location 0: Position
            vkCommon::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 1,
                                                                    VK_FORMAT_R32G32B32_SFLOAT,
                                                                    sizeof(float) * 3), // Location 1: Normal
            vkCommon::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 2, VK_FORMAT_R32G32_SFLOAT,
                                                                    sizeof(float) *
                                                                    6), // Location 2: Texture coordinates
            vkCommon::initializers::vertexInputAttributeDescription(VERTEX_BUFFER_BIND_ID, 3,
                                                                    VK_FORMAT_R32G32B32_SFLOAT,
                                                                    sizeof(float) * 8), // Location 3: Color
            // Per-Instance attributes
            // These are fetched for each instance rendered
            vkCommon::initializers::vertexInputAttributeDescription(INSTANCE_BUFFER_BIND_ID, 4,
                                                                    VK_FORMAT_R32G32B32_SFLOAT,
                                                                    0), // Location 4: Position
            vkCommon::initializers::vertexInputAttributeDescription(INSTANCE_BUFFER_BIND_ID, 5,
                                                                    VK_FORMAT_R32G32B32_SFLOAT,
                                                                    sizeof(float) * 3), // Location 5: Rotation
            vkCommon::initializers::vertexInputAttributeDescription(INSTANCE_BUFFER_BIND_ID, 6, VK_FORMAT_R32_SFLOAT,
                                                                    sizeof(float) * 6), // Location 6: Scale
            vkCommon::initializers::vertexInputAttributeDescription(INSTANCE_BUFFER_BIND_ID, 7, VK_FORMAT_R32_SINT,
                                                                    sizeof(float) *
                                                                    7), // Location 7: Texture array layer index
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) renderContext->getSwapChainExtent().width;
    viewport.height = (float) renderContext->getSwapChainExtent().height;
    viewport.minDepth = 0;
    viewport.maxDepth = 1.0;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = renderContext->getSwapChainExtent();

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;
    viewportState.pNext = nullptr;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;
    rasterizer.pNext = nullptr;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType =
            VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                          VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                                          VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType =
            VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;

    pipelineLayoutCreateInfo.setLayoutCount = 1;
    VkDescriptorSetLayout descriptorSetLayouts[] = {descriptorLayout->getHandle()};
    pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts;

    if (vkCreatePipelineLayout(device->getHandle(), &pipelineLayoutCreateInfo, nullptr, &pipelineLayOut) !=
        VK_SUCCESS)
        RUN_TIME_ERROR("Failed to create pipeline layout");

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0;
    depthStencil.maxDepthBounds = 1.0;

    std::vector<VkDynamicState> dynamicStateEnables;
    dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStateEnables.data();

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    // pipelineInfo.pDepthStencilState = nullptr;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr;
    pipelineInfo.layout = pipelineLayOut;

    pipelineInfo.renderPass = renderPipeline->getRenderPass().getHandle();
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    // init instanced rock pipeline
    auto vertexShader = std::make_unique<Shader>(*device, FileUtils::getShaderPath() + "instancing.vert");
    auto fragShader = std::make_unique<Shader>(*device, FileUtils::getShaderPath() + "instancing.frag");
    shaderStages[0] = vertexShader->PipelineShaderStageCreateInfo();
    shaderStages[1] = fragShader->PipelineShaderStageCreateInfo();

    vertexInputInfo.vertexBindingDescriptionCount = bindingDescriptions.size();
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VK_CHECK_RESULT(
            vkCreateGraphicsPipelines(device->getHandle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                      &pipelines.instancedRockPipeline))

    // init planet pipeline

    vertexShader = std::make_unique<Shader>(*device, FileUtils::getShaderPath("planet.vert"));
    fragShader = std::make_unique<Shader>(*device, FileUtils::getShaderPath("planet.frag"));
    shaderStages[0] = vertexShader->PipelineShaderStageCreateInfo();
    shaderStages[1] = fragShader->PipelineShaderStageCreateInfo();
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    // pos normal uv color
    vertexInputInfo.vertexAttributeDescriptionCount = 4;
    VK_CHECK_RESULT(
            vkCreateGraphicsPipelines(device->getHandle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                      &pipelines.planetPipeline))
}

void Example::prepare() {
    Application::prepare();

    loadResources();
    prepareInstanceData();
    prepareUniformBuffers();

    createDescriptorSetLayout();
    createDescriptorPool();
    createDescriptorSet();

    createGraphicsPipeline();

    buildCommandBuffers();
}

void Example::createDescriptorSetLayout() {
    descriptorLayout = std::make_unique<DescriptorLayout>(*device);
    descriptorLayout->addBinding(VK_SHADER_STAGE_VERTEX_BIT, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0);
    descriptorLayout->addBinding(VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0);
    descriptorLayout->createLayout(0);
}

Example::Example() : Application("Instancing", 1280, 720) {
    camera = std::make_unique<Camera>();
    camera->flipY = true;
    camera->setPerspective(45.f, 1.f, 0.1f, 10.0f);
    camera->setRotation(glm::vec3(0));
    camera->setTranslation(glm::vec3(0.f, 0.f, -2.f));
    camera->setTranslation(glm::vec3(5.5f, -1.85f, -18.5f));
    camera->setRotation(glm::vec3(-17.2f, -4.7f, 0.0f));
    camera->setPerspective(60.0f, (float) width / (float) height, 1.0f, 256.0f);

    sponza = std::make_unique<gltfLoading::GLTFLoadingImpl>(*device);
    sponza->loadFromFile(FileUtils::getResourcePath("sponza/sponza.gltf"));
}

void Example::bindUniformBuffers(CommandBuffer &commandBuffer) {
    // commandBuffer.bindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayOut, 0, {descriptorSet.get()},
    //                                  {});
}

void Example::buildCommandBuffers() {
    for (int i = commandBuffers.size() - 1; i >= 0; i--) {
        auto &commandBuffer = *commandBuffers[i];
        commandBuffer.beginRecord(0);
        //    commandBuffer.bindPipeline(graphicsPipeline->getHandle());
        renderContext->setActiveFrameIdx(i);
        //        bindUniformBuffers(commandBuffer);
        draw(commandBuffer);
        commandBuffer.endRecord();
    }
}

void Example::onUpdateGUI() {
    gui->text("Hello");
    gui->text("Hello IMGUI");
    gui->text("Hello imgui");
}

void Example::prepareInstanceData() {
    std::vector<InstanceData> instanceData;
    instanceData.resize(INSTANCE_NUM);

    std::default_random_engine rndGenerator(0);
    std::uniform_real_distribution<float> uniformDist(0.0, 1.0);
    std::uniform_int_distribution<uint32_t> rndTextureIndex(0, textures.rocketTexture.image->getLayers());

    // Distribute rocks randomly on two different rings
    for (auto i = 0; i < INSTANCE_NUM / 2; i++) {
        glm::vec2 ring0{7.0f, 11.0f};
        glm::vec2 ring1{14.0f, 18.0f};

        float rho, theta;

        // Inner ring
        rho = sqrt((pow(ring0[1], 2.0f) - pow(ring0[0], 2.0f)) * uniformDist(rndGenerator) + pow(ring0[0], 2.0f));
        theta = 2.0 * M_PI * uniformDist(rndGenerator);
        instanceData[i].pos = glm::vec3(rho * cos(theta), uniformDist(rndGenerator) * 0.5f - 0.25f, rho * sin(theta));
        instanceData[i].rot = glm::vec3(M_PI * uniformDist(rndGenerator), M_PI * uniformDist(rndGenerator),
                                        M_PI * uniformDist(rndGenerator));
        instanceData[i].scale = 1.5f + uniformDist(rndGenerator) - uniformDist(rndGenerator);
        instanceData[i].texIndex = rndTextureIndex(rndGenerator);
        instanceData[i].scale *= 0.75f;

        // Outer ring
        rho = sqrt((pow(ring1[1], 2.0f) - pow(ring1[0], 2.0f)) * uniformDist(rndGenerator) + pow(ring1[0], 2.0f));
        theta = 2.0 * M_PI * uniformDist(rndGenerator);
        instanceData[i + INSTANCE_NUM / 2].pos = glm::vec3(rho * cos(theta), uniformDist(rndGenerator) * 0.5f - 0.25f,
                                                           rho * sin(theta));
        instanceData[i + INSTANCE_NUM / 2].rot = glm::vec3(M_PI * uniformDist(rndGenerator),
                                                           M_PI * uniformDist(rndGenerator),
                                                           M_PI * uniformDist(rndGenerator));
        instanceData[i + INSTANCE_NUM / 2].scale = 1.5f + uniformDist(rndGenerator) - uniformDist(rndGenerator);
        instanceData[i + INSTANCE_NUM / 2].texIndex = rndTextureIndex(rndGenerator);
        instanceData[i + INSTANCE_NUM / 2].scale *= 0.75f;
    }

    instanceBuffer.buffer = std::make_unique<Buffer>(*device, sizeof(InstanceData) * instanceData.size(),
                                                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                                     VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                     VMA_MEMORY_USAGE_CPU_TO_GPU);

    instanceBuffer.buffer->uploadData(instanceData.data(), DATA_SIZE(instanceData));
}

void Example::draw(CommandBuffer &commandBuffer) {
    bindUniformBuffers(commandBuffer);
    //    renderPipeline->draw(commandBuffer, renderFrame);

    commandBuffer.beginRenderPass(renderPipeline->getRenderPass(),
                                  Default::clearValues(), VkSubpassContents{});

    const VkViewport viewport = vkCommon::initializers::viewport((float) width, (float) height, 0.0f, 1.0f);
    const VkRect2D scissor = vkCommon::initializers::rect2D(width, height, 0, 0);
    vkCmdSetViewport(commandBuffer.getHandle(), 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer.getHandle(), 0, 1, &scissor);

    commandBuffer.bindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayOut, 0,
                                     {*descriptors.planetDescriptor},
                                     {});
    vkCmdBindPipeline(commandBuffer.getHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.planetPipeline);
    models.planets->draw(commandBuffer);

    commandBuffer.bindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayOut, 0,
                                     {*descriptors.rockDescriptor},
                                     {});

    VkDeviceSize offsets[1] = {0};

    vkCmdBindPipeline(commandBuffer.getHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.instancedRockPipeline);
    //   models.rockets->draw(commandBuffer);

    // vkCmdBindVertexBuffers(commandBuffer.getHandle(), VERTEX_BUFFER_BIND_ID, 1, &models.rockets->vertices->getHandle(),
    //                        offsets);

    // Binding point 1 : Instance data buffer
    //  vkCmdBindVertexBuffers(commandBuffer.getHandle(), INSTANCE_BUFFER_BIND_ID, 1, &instanceBuffer.buffer->getHandle(),
    //                       offsets);

    // Bind index buffer
    vkCmdBindIndexBuffer(commandBuffer.getHandle(), models.rockets->indices->getHandle(), 0, VK_INDEX_TYPE_UINT32);

    // Render instances
    vkCmdDrawIndexed(commandBuffer.getHandle(), models.rockets->indexCount, INSTANCE_NUM, 0, 0, 0);
    gui->draw(commandBuffer.getHandle());

    commandBuffer.endRenderPass();
}

int main() {
    auto example = new Example();
    example->prepare();
    example->mainloop();
    return 0;
}
