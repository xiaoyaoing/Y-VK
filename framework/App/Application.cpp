//
// Created by 打工人 on 2023/3/19.
//
#include "Application.h"
#include "Instance.h"
#include "Scene.h"
#include <RenderTarget.h>
#include <Shader.h>
#include <Subpass.h>

void Application::initVk() {
    getRequiredExtensions();
    _instance = std::make_unique<Instance>(std::string("vulkanApp"), instanceExtensions, validationLayers);
    surface = window->createSurface(*_instance);

    // create device
    uint32_t physicalDeviceCount;
    VkPhysicalDevice *physicalDevices;
    vkEnumeratePhysicalDevices(_instance->getHandle(), &physicalDeviceCount, physicalDevices);
    auto physicalDevice = physicalDevices[0];
    device = std::make_unique<Device>(physicalDevice, surface);

    createAllocator();
    createRenderContext();
    createCommandBuffer();
    //createRenderPass();
    createRenderPipeline();
    createPipeline();
    // createDepthStencil();

    renderContext->createFrameBuffers(renderPipeline->getRenderPass());
    //  createFrameBuffers();
}

void Application::getRequiredExtensions() {
    uint32_t glfwExtensionsCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);
    for (uint32_t i = 0; i < glfwExtensionsCount; i++) {
        addInstanceExtension(glfwExtensions[i]);
    }
    if (enableValidationLayers)
        addInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    addInstanceExtension(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
}

//void Application::createPipeline() {
//    VkAttachmentDescription colorAttachment{};
//    colorAttachment.format = renderContext->getSwapChainFormat();
//    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
//
//    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//    //
//    VkAttachmentDescription depthAttachment{};
//    depthAttachment.format = findSupportedFormat(
//            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
//            VK_IMAGE_TILING_OPTIMAL,
//            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
//    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
//    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//
//    VkAttachmentReference colorAttachmentRef{};
//    colorAttachmentRef.attachment = 0;
//    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//
//    VkAttachmentReference depthAttachmentRef{};
//    depthAttachmentRef.attachment = 1;
//    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
//
//    VkSubpassDescription subpass{};
//    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//    subpass.colorAttachmentCount = 1;
//    subpass.pColorAttachments = &colorAttachmentRef;
//    subpass.pDepthStencilAttachment = &depthAttachmentRef;
//
//    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
//
//    VkSubpassDependency dependency{};
//    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
//    dependency.dstSubpass = 0;
//    dependency.srcStageMask =
//            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT & VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
//    dependency.srcAccessMask = 0;
//    dependency.dstStageMask =
//            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT & VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
//    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT & VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
//
//    VkRenderPassCreateInfo renderPassInfo{};
//    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
//    renderPassInfo.attachmentCount = attachments.size();
//    renderPassInfo.pAttachments = attachments.data();
//    renderPassInfo.subpassCount = 1;
//    renderPassInfo.pSubpasses = &subpass;
//    renderPassInfo.dependencyCount = 1;
//    renderPassInfo.pDependencies = &dependency;
//    VkRenderPass _pass;
//    auto result = vkCreateRenderPass(device->getHandle(), &renderPassInfo, nullptr, &_pass);
//    if (result != VK_SUCCESS) {
//        RUN_TIME_ERROR("Failed to create render pass")
//    }
//    renderPass = std::make_shared<RenderPass>(_pass);
//}

void Application::createPipeline() {
    // auto vertShaderCode = readFile(parentPath + "shaders/vert.spv");
    // auto fragShaderCode = readFile(parentPath + "shaders/frag.spv");

    // auto vertShaderModule = createShaderModule(vertShaderCode);
    // auto fragShaderModule = createShaderModule(fragShaderCode);

    // VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    // vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    // vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    // vertShaderStageInfo.module = vertShaderModule;
    // vertShaderStageInfo.pName = "main";

    // VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    // fragShaderStageInfo.sType =
    //     VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    // fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    // fragShaderStageInfo.module = fragShaderModule;
    // fragShaderStageInfo.pName = "main";

    // todo handle shader complie
    auto vertexShader = Shader(*device, "E:\\code\\VulkanFrameWorkLearn\\shaders\\vert.spv");
    auto fragShader = Shader(*device, "E:\\code\\VulkanFrameWorkLearn\\shaders\\frag.spv");

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertexShader.PipelineShaderStageCreateInfo(),
                                                      fragShader.PipelineShaderStageCreateInfo()};

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
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
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
    pipelineLayoutCreateInfo.setLayoutCount = 0;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    VkDescriptorSetLayout pSetLayouts[] = {descriptorSetLayout};
    pipelineLayoutCreateInfo.pSetLayouts = pSetLayouts;
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
    if (vkCreateGraphicsPipelines(device->getHandle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }
    graphicsPipeline = std::make_unique<Pipeline>(pipeline);

}

void Application::updateScene() {

}

void Application::updateGUI() {

}

void Application::createFrameBuffers() {

//    // DestroyFrameBuffers();
//    uint32_t width = _context->getSwapChainExtent().width;
//    uint32_t height = _context->getSwapChainExtent().height;
//
//    VkImageView attachments[2];
//    attachments[1] = _depthImageView->getHandle();
//
//    VkFramebufferCreateInfo frameBufferCreateInfo;
//    frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//    frameBufferCreateInfo.renderPass = _renderPass->getHandle();
//    frameBufferCreateInfo.attachmentCount = 2;
//    frameBufferCreateInfo.pAttachments = attachments;
//    frameBufferCreateInfo.width = width;
//    frameBufferCreateInfo.height = height;
//    frameBufferCreateInfo.layers = 1;
//
//    const std::vector<VkImageView> &backBufferViews = _context->getBackBufferViews();
//
//    _frameBuffers.resize(backBufferViews.size());
//    for (uint32_t i = 0; i < _frameBuffers.size(); ++i) {
//        attachments[0] = backBufferViews[i];
//        VK_CHECK_RESULT(vkCreateFramebuffer(device->getHandle(), &frameBufferCreateInfo, nullptr, &_frameBuffers[i]));
//    }
}

void Application::createCommandBuffer() {
    commandPool = std::make_unique<CommandPool>(*device, _graphicsQueue->getFamilyIndex(),
                                                CommandBuffer::ResetMode::AlwaysAllocate);

    commandBuffers.reserve(_context->getSwapChainImageCount());
    VkCommandBufferAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = commandPool->getHandle();
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = commandBuffers.size();

    std::vector<VkCommandBuffer> vkCommandBuffers(commandBuffers.size());

    if (vkAllocateCommandBuffers(device->getHandle(), &allocateInfo, vkCommandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
    for (const auto &vkCommandBuffer: vkCommandBuffers)
        commandBuffers.emplace_back(std::move(std::make_unique<CommandBuffer>(vkCommandBuffer)));
}

void Application::createRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = _context->getSwapChainFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    VkAttachmentDescription depthAttachment{};

    depthAttachment.format = VK_FORMAT_D32_SFLOAT;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT & VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT & VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT & VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::vector<VkAttachmentDescription> attachments = {colorAttachment, depthAttachment};
    std::vector<VkSubpassDescription> subpasses = {subpass};
    std::vector<VkSubpassDependency> dependencies = {dependency};

//    _renderPass = std::make_shared<RenderPass>(device, attachments, dependencies, subpasses);
}

void Application::createDepthStencil() {
//    auto depthImageInfo = Image::getDefaultImageInfo();
//    depthImageInfo.extent = VkExtent3D{_context->getSwapChainExtent().width, _context->getSwapChainExtent().height, 1};
//    depthImageInfo.format = VK_FORMAT_D32_SFLOAT;
//    depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
//    depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
//    depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
//    _depthImage = std::make_shared<Image>(_allocator, VMA_MEMORY_USAGE_GPU_ONLY, depthImageInfo);
//    _depthImageView = std::make_shared<ImageView>(device, _depthImage, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

void Application::createAllocator() {
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = device->getPhysicalDevice();
    allocatorInfo.device = device->getHandle();
    allocatorInfo.instance = _instance->getHandle();

    VK_CHECK_RESULT(vmaCreateAllocator(&allocatorInfo, &_allocator));
    device->setMemoryAllocator(_allocator);
}

void Application::update() {
    auto commandBuffer = _context->begin();
}

void Application::draw(CommandBuffer &commandBuffer, RenderTarget &renderTarget) {
    auto &views = renderTarget.getViews();

    auto swapchain_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    {
        ImageMemoryBarrier memory_barrier{};
        memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        memory_barrier.newLayout = swapchain_layout;
        memory_barrier.srcAccessMask = 0;
        memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        memory_barrier.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        memory_barrier.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        for (auto &i: colorIdx) {
            assert(i < views.size());
            commandBuffer.imageMemoryBarrier(views[i], memory_barrier);
            renderTarget.setLayout(i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        }
    }

    {
        ImageMemoryBarrier memory_barrier{};
        memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        memory_barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        memory_barrier.srcAccessMask = 0;
        memory_barrier.dstAccessMask =
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        memory_barrier.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        memory_barrier.dstStageMask =
                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

        for (auto &i: depthIdx) {
            assert(i < views.size());
            commandBuffer.imageMemoryBarrier(views[i], memory_barrier);
            renderTarget.setLayout(i, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        }

        renderPipeline->draw(commandBuffer, renderTarget);
        commandBuffer.endRenderPass();
    }
}

void Application::createRenderContext() {
    auto surface_priority_list = std::vector<VkSurfaceFormatKHR>{{VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
                                                                 {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}};
    renderContext = std::make_unique<RenderContext>(*device, surface, *window);
}

void Application::drawFrame() {
    updateScene();
    updateGUI();
//    auto &commandBuffer = renderContext->begin();
    renderContext->beginFrame();
    auto &commandBuffer = *commandBuffers[renderContext->getActiveFrameIndex()];
    commandBuffer.beginRecord(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    draw(commandBuffer, renderContext->getActiveRenderFrame().getRenderTarget());
    commandBuffer.endRecord();
    renderContext->submit(commandBuffer);
}

void Application::drawRenderPasses(CommandBuffer &buffer, RenderTarget &renderTarget) {
    renderPipeline->draw(buffer, renderTarget);
}

void Application::initWindow(const char *name, int width, int height) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = std::make_unique<Window>(glfwCreateWindow(width, height, name, nullptr, nullptr));
    glfwSetFramebufferSizeCallback(window->getHandle(), [](GLFWwindow *window, int width, int height) {
        auto *app = reinterpret_cast<Application *>(glfwGetWindowUserPointer(window));
        app->frameBufferResized = true;
    });
}

void Application::initGUI() {

}

void Application::createCommandPool() {

}

VkPhysicalDevice Application::createPhysicalDevice() {
    return nullptr;
}

void Application::prepare() {
    initGUI();
    initVk();
}

Application::Application(const char *name, int width, int height) {
    initWindow(name, width, height);
}

void Application::createRenderPipeline() {
    scene = std::make_unique<Scene>(*device);
    renderPipeline = std::make_unique<RenderPipeline>(*device);
    renderPipeline->addSubPass(std::make_unique<GeomSubpass>(*scene));
    std::vector<LoadStoreInfo> infos;
    infos.emplace_back(LoadStoreInfo{VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE});
    infos.emplace_back(LoadStoreInfo{VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE});
    renderPipeline->createRenderPass(renderContext->getRenderFrame(0).getRenderTarget(), infos);
}
