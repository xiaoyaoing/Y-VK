//
// Created by pc on 2023/8/17.
//

#include "sample1.h"
#include "Shader.h"
#include "Common/VulkanInitializers.h"
#include "FIleUtils.h"

void sample1::prepareUniformBuffers() {
    uniform_buffers.scene = std::make_unique<Buffer>(*device, sizeof(ubo_vs), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                     VMA_MEMORY_USAGE_CPU_TO_GPU);
    uniform_buffers.scene->uploadData(&ubo_vs, sizeof(ubo_vs));
}

void sample1::createDescriptorSet() {
    descriptorSet = std::make_unique<DescriptorSet>(*device, *descriptorPool, *descriptorLayout, 1);
    descriptorSet->updateBuffer({uniform_buffers.scene.get()}, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

    textures.texture1 = loadTexture(FileUtils::getResourcePath() + "Window.png");

    auto imageDescriptorInfo = VkCommon::DescriptorImageInfo(textures.texture1);

    descriptorSet->updateImage({imageDescriptorInfo}, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
}

void sample1::createDescriptorPool() {

    std::vector<VkDescriptorPoolSize> poolSizes = {
            VkCommon::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2),
            VkCommon::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2)};

    descriptorPool = std::make_unique<DescriptorPool>(*device, poolSizes, 2);
}

void sample1::updateUniformBuffers() {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    ubo_vs.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo_vs.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo_vs.proj = glm::perspective(glm::radians(45.0f), 1.f, 0.1f, 10.0f);
    ubo_vs.proj[1][1] *= -1;
    // ubo_vs.model = glm::mat4::Ide
    uniform_buffers.scene->uploadData(&ubo_vs, sizeof(ubo_vs));
}

void sample1::createGraphicsPipeline() {

    // todo handle shader complie
    auto vertexShader = Shader(*device, FileUtils::getShaderPath() + "vert.spv");
    auto fragShader = Shader(*device, FileUtils::getShaderPath() + "frag.spv");

    VkPipelineShaderStageCreateInfo shaderStages[] = {
            vertexShader.PipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT),
            fragShader.PipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT)};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

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

    VkPipeline pipeline;
    VK_CHECK_RESULT(
            vkCreateGraphicsPipelines(device->getHandle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline));
    graphicsPipeline = std::make_unique<Pipeline>(pipeline);
}

void sample1::prepare() {
    Application::prepare();

    prepareUniformBuffers();
    createDescriptorSetLayout();
    createDescriptorPool();
    createDescriptorSet();

    createGraphicsPipeline();

    buildCommandBuffers();
}


void sample1::createDescriptorSetLayout() {
    descriptorLayout = std::make_unique<DescriptorLayout>(*device);
    descriptorLayout->addBinding(VK_SHADER_STAGE_VERTEX_BIT, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0);
    descriptorLayout->addBinding(VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0);
    descriptorLayout->createLayout(0);
}

sample1::sample1(const char *name, int width, int height) : Application(name, width, height) {
}

void sample1::bindUniformBuffers(CommandBuffer &commandBuffer) {
    commandBuffer.bindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayOut, 0, {descriptorSet.get()},
                                     {});
}

void sample1::buildCommandBuffers() {
    for (int i = commandBuffers.size() - 1; i >= 0; i--) {
        auto &commandBuffer = *commandBuffers[i];
        commandBuffer.beginRecord(0);
        commandBuffer.bindPipeline(graphicsPipeline->getHandle());
        renderContext->setActiveFrameIdx(i);
        bindUniformBuffers(commandBuffer);
        draw(commandBuffer, renderContext->getRenderFrame(i));
        commandBuffer.endRecord();
    }
}
