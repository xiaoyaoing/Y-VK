//
// Created by 打工人 on 2023/3/19.
//
#include "Application.h"
#include "Instance.h"
#include <RenderTarget.h>

void Application::initVk() {
    getRequiredExtensions();
    _instance = std::make_unique<Instance>(std::string("vulkanApp"), instanceExtensions, validationLayers);
    surface = _window->createSurface(*_instance);
    auto physicalDevice = createPhysicalDevice();
    _device = std::make_shared<Device>(physicalDevice, surface);
    _graphicsQueue = _device->getQueueByFlag(VK_QUEUE_GRAPHICS_BIT, 0);
    _presentQueue = _device->getPresentQueue(0);
    createAllocator();
    createRenderContext();
    createCommandBuffer();
    createRenderPass();
    createDepthStencil();
    createFrameBuffers();
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

void Application::createFrameBuffers() {

    // DestroyFrameBuffers();
    uint32_t width = _context->getSwapChainExtent().width;
    uint32_t height = _context->getSwapChainExtent().height;

    VkImageView attachments[2];
    attachments[1] = _depthImageView->getHandle();

    VkFramebufferCreateInfo frameBufferCreateInfo;
    frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frameBufferCreateInfo.renderPass = _renderPass->getHandle();
    frameBufferCreateInfo.attachmentCount = 2;
    frameBufferCreateInfo.pAttachments = attachments;
    frameBufferCreateInfo.width = width;
    frameBufferCreateInfo.height = height;
    frameBufferCreateInfo.layers = 1;

    const std::vector<VkImageView> &backBufferViews = _context->getBackBufferViews();

    _frameBuffers.resize(backBufferViews.size());
    for (uint32_t i = 0; i < _frameBuffers.size(); ++i) {
        attachments[0] = backBufferViews[i];
        VK_VERIFY_RESULT(vkCreateFramebuffer(_device->getHandle(), &frameBufferCreateInfo, nullptr, &_frameBuffers[i]));
    }
}

void Application::createCommandBuffer() {
    commandPool = std::make_shared<CommandPool>(_device, _graphicsQueue,
                                                VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    commandBuffers.reserve(_context->getBackBufferCount());
    VkCommandBufferAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = commandPool->getHandle();
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = commandBuffers.size();

    std::vector<VkCommandBuffer> vkCommandBuffers(commandBuffers.size());

    if (vkAllocateCommandBuffers(_device->getHandle(), &allocateInfo, vkCommandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
    for (const auto &vkCommandBuffer: vkCommandBuffers)
        commandBuffers.emplace_back(std::make_shared<CommandBuffer>(vkCommandBuffer));
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

    _renderPass = std::make_shared<RenderPass>(_device, attachments, dependencies, subpasses);
}

void Application::createDepthStencil() {
    auto depthImageInfo = Image::getDefaultImageInfo();
    depthImageInfo.extent = VkExtent3D{_context->getSwapChainExtent().width, _context->getSwapChainExtent().height, 1};
    depthImageInfo.format = VK_FORMAT_D32_SFLOAT;
    depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
    depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    _depthImage = std::make_shared<Image>(_allocator, VMA_MEMORY_USAGE_GPU_ONLY, depthImageInfo);
    _depthImageView = std::make_shared<ImageView>(_device, _depthImage, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

void Application::createAllocator() {
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = _device->getPhysicalDevice();
    allocatorInfo.device = _device->getHandle();
    allocatorInfo.instance = _instance->getHandle();

    ASSERT(vmaCreateAllocator(&allocatorInfo, &_allocator) == VK_SUCCESS, "create allocator");
    _device->setMemoryAllocator(_allocator);
}

void Application::update() {
    auto commandBuffer = _context->begin();
}

void Application::draw(CommandBuffer &commandBuffer, RenderTarget &renderTarget) {
    auto &views = renderTarget.getViews();
    assert(1 < views.size());

    {
        ImageMemoryBarrier memoryBarrier;
        // Image 0 is the swapchain
        memoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        memoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        memoryBarrier.srcAccessMask = 0;
        memoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        memoryBarrier.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        memoryBarrier.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        commandBuffer.imageMemoryBarrier(views[0], memoryBarrier);

        // Skip 1 as it is handled later as a depth-stencil attachment
        for (size_t i = 2; i < views.size(); ++i) {
            commandBuffer.imageMemoryBarrier(views[i], memoryBarrier);
        }
    }
    drawRenderPasses(commandBuffer, renderTarget);
    {
        ImageMemoryBarrier memoryBarrier{};
        memoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        memoryBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        memoryBarrier.srcAccessMask = 0;
        memoryBarrier.dstAccessMask =
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        memoryBarrier.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        memoryBarrier.dstStageMask =
                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

        commandBuffer.imageMemoryBarrier(views[1], memoryBarrier);
    }

    drawRenderPasses(commandBuffer, renderTarget);

    {
        ImageMemoryBarrier memoryBarrier{};
        memoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        memoryBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        memoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        memoryBarrier.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        memoryBarrier.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

        commandBuffer.imageMemoryBarrier(views[0], memoryBarrier);
    }
}

void Application::createRenderContext() {
    auto surface_priority_list = std::vector<VkSurfaceFormatKHR>{{VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
                                                                 {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}};
    renderContext = std::make_unique<RenderContext>(*device, surface, *_window);
}

void Application::present() {
}

void Application::drawFrame() {
    updateScene();
    updateGUI();
    auto &commandBuffer = this->renderContext->begin();
    commandBuffer.beginRecord(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    draw(commandBuffer, renderContext->getActiveRenderFrame().getRenderTarget());
    commandBuffer.endRecord();
    renderContext->submit(commandBuffer);
}

void Application::drawRenderPasses(CommandBuffer &buffer, RenderTarget &renderTarget) {
    //set_viewport_and_scissor(command_buffer, render_target.get_extent());

    renderPipeline->draw()

    //todo handle GUI
//    if (gui)
//    {
//        gui->draw(command_buffer);
//    }

    //command_buffer.end_render_pass();
}
