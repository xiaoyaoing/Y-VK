//
// Created by 打工人 on 2023/3/19.
//
#include "Application.h"
#include "Instance.h"
#include "Scene.h"
#include "Common/VulkanInitializers.h"
#include <RenderTarget.h>
#include <Shader.h>
#include <Subpass.h>
#include <API_VK.h>
#include <Camera.h>


void Application::initVk() {
    getRequiredInstanceExtensions();
    _instance = std::make_unique<Instance>(std::string("vulkanApp"), instanceExtensions, validationLayers);
    surface = window->createSurface(*_instance);

    // create device
    uint32_t physical_device_count{0};
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(_instance->getHandle(), &physical_device_count, nullptr));

    if (physical_device_count < 1) {
        throw std::runtime_error("Couldn't find a physical device that supports Vulkan.");
    }

    std::vector<VkPhysicalDevice> physical_devices;
    physical_devices.resize(physical_device_count);

    LOGI("Found {} physical device", physical_device_count);

    VK_CHECK_RESULT(
            vkEnumeratePhysicalDevices(_instance->getHandle(), &physical_device_count, physical_devices.data()));

    addDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physical_devices[0], &deviceProperties);
    LOGI("Device Name: {}", deviceProperties.deviceName)

    device = std::make_unique<Device>(physical_devices[0], surface, deviceExtensions);

    createAllocator();

    createRenderContext();
    createRenderPipeline();

    createRenderPass();

    // createPipeline();
    //  createDepthStencil();

    renderContext->createFrameBuffers(renderPipeline->getRenderPass());

    createCommandBuffer();

    //  createFrameBuffers();

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    VK_CHECK_RESULT(vkCreateFence(device->getHandle(), &fenceInfo, nullptr, &fence));
}

void Application::getRequiredInstanceExtensions() {
    uint32_t glfwExtensionsCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);
    for (uint32_t i = 0; i < glfwExtensionsCount; i++) {
        addInstanceExtension(glfwExtensions[i]);
    }
    if (enableValidationLayers)
        addInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    //  addInstanceExtension(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
}

void Application::updateScene() {
}

void Application::updateGUI() {
}

void Application::createFrameBuffers() {
}

void Application::createCommandBuffer() {
    commandPool = std::make_unique<CommandPool>(*device,
                                                device->getQueueByFlag(VK_QUEUE_GRAPHICS_BIT, 0).getFamilyIndex(),
                                                CommandBuffer::ResetMode::AlwaysAllocate);

    auto frameCount = renderContext->getSwapChainImageCount();
    commandBuffers.reserve(frameCount);
    VkCommandBufferAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = commandPool->getHandle();
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = frameCount;
    std::vector<VkCommandBuffer> vkCommandBuffers(frameCount);

    if (vkAllocateCommandBuffers(device->getHandle(), &allocateInfo, vkCommandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
    for (const auto &vkCommandBuffer: vkCommandBuffers)
        commandBuffers.emplace_back(std::move(std::make_unique<CommandBuffer>(vkCommandBuffer)));
}

void Application::createRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = renderContext->getSwapChainFormat();
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
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = nullptr;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = nullptr;
    subpass.pResolveAttachments = nullptr;

    std::array<VkSubpassDependency, 2> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    std::vector<VkAttachmentDescription> attachments = {colorAttachment, depthAttachment};
    std::vector<VkSubpassDescription> subpasses = {subpass};

    VkRenderPassCreateInfo render_pass_create_info = {};
    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
    render_pass_create_info.pAttachments = attachments.data();
    render_pass_create_info.subpassCount = 1;
    render_pass_create_info.pSubpasses = &subpass;
    render_pass_create_info.dependencyCount = static_cast<uint32_t>(dependencies.size());
    render_pass_create_info.pDependencies = dependencies.data();

    VkRenderPass vkRenderPass;
    VK_CHECK_RESULT(vkCreateRenderPass(device->getHandle(), &render_pass_create_info, nullptr, &vkRenderPass));

    renderPipeline->createRenderPass(vkRenderPass);
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
    auto tStart = std::chrono::high_resolution_clock::now();

    updateScene();
    updateGUI();

    drawFrame();

    auto tEnd = std::chrono::high_resolution_clock::now();

    auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();

    camera->update(tDiff / 1000.f);
}

// HWND Application::setupWindow(HINSTANCE hinstance, WNDPROC wndproc)
// {
//     return HWND();
// }

// void Application::handleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
// {
// }

void Application::draw(CommandBuffer &commandBuffer, RenderFrame &renderFrame) {
    auto &renderTarget = renderFrame.getRenderTarget();
    auto &views = renderTarget.getViews();
    bindUniformBuffers(commandBuffer);
    renderPipeline->draw(commandBuffer, renderFrame);
    commandBuffer.endRenderPass();

    //    {
    //        ImageMemoryBarrier memory_barrier{};
    //        memory_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    //        memory_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    //        memory_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    //        memory_barrier.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //        memory_barrier.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    //
    //        commandBuffer.imageMemoryBarrier(views[0], memory_barrier);
    //    }
}

void Application::createRenderContext() {
    auto surface_priority_list = std::vector<VkSurfaceFormatKHR>{{VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
                                                                 {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}};
    renderContext = std::make_unique<RenderContext>(*device, surface, *window);
    RenderContext::g_context = renderContext.get();
}

void Application::drawFrame() {
    vkWaitForFences(device->getHandle(), 1, &fence, VK_TRUE, UINT64_MAX);
    vkResetFences(device->getHandle(), 1, &fence);
    renderContext->beginFrame();
    auto &commandBuffer = *commandBuffers[renderContext->getActiveFrameIndex()];
    renderContext->submit(commandBuffer, fence);
}

void Application::drawRenderPasses(CommandBuffer &buffer, RenderTarget &renderTarget) {
    //    renderPipeline->draw(buffer, renderTarget);
}

void Application::initWindow(const char *name, int width, int height) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = std::make_unique<Window>(Window::WindowProp{"title"}, this);
//    glfwSetFramebufferSizeCallback(window->getHandle(), [](GLFWwindow *window, int width, int height) {
//        auto *app = reinterpret_cast<Application *>(glfwGetWindowUserPointer(window));
//        app->frameBufferResized = true;
//    });
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
}

Texture Application::loadTexture(const std::string &path) {
    Texture texture{};
    texture.image = sg::SgImage::load(path);
    texture.image->createVkImage(*device);

    Buffer imageBuffer = Buffer(*device, texture.image->getBufferSize(),
                                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                VMA_MEMORY_USAGE_CPU_ONLY);
    imageBuffer.uploadData(static_cast<void *>(texture.image->getData().data()), texture.image->getBufferSize());

    VkBufferImageCopy imageCopy{};
    imageCopy.bufferOffset = 0;
    imageCopy.bufferRowLength = 0;
    imageCopy.bufferImageHeight = 0;
    imageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageCopy.imageSubresource.mipLevel = 0;
    imageCopy.imageSubresource.baseArrayLayer = 0;
    imageCopy.imageSubresource.layerCount = 1;
    imageCopy.imageOffset = {0, 0, 0};
    imageCopy.imageExtent = {texture.image->getExtent().width, texture.image->getExtent().height, 1};

    auto commandBuffer = commandPool->allocateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    VkCommon::setImageLayout(commandBuffer.getHandle(), texture.image->getVkImage().getHandle(),
                             VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    commandBuffer.copyBufferToImage(imageBuffer, texture.image->getVkImage(), {imageCopy});
    VkCommon::setImageLayout(commandBuffer.getHandle(), texture.image->getVkImage().getHandle(),
                             VK_FORMAT_R8G8B8A8_SRGB,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    commandBuffer.endRecord();

    auto queue = device->getQueueByFlag(VK_QUEUE_GRAPHICS_BIT, 0);

    VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;

    auto vkCmdBuffer = commandBuffer.getHandle();

    submitInfo.pCommandBuffers = &vkCmdBuffer;
    queue.submit({submitInfo}, VK_NULL_HANDLE);

    texture.sampler = std::make_unique<Sampler>(*device, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_FILTER_LINEAR, 1);

    return texture;
}

void Application::inputEvent(const InputEvent &inputEvent) {
    auto source = inputEvent.getSource();

    if (source == EventSource::KeyBoard) {
        const auto &keyEvent = static_cast<const KeyInputEvent &>(inputEvent);
        auto action = keyEvent.getAction();
        auto code = keyEvent.getCode();

        switch (action) {

            case KeyAction::Down:
                switch (code) {
                    case KeyCode::W:
                        camera->keys.up = true;
                        break;
                    case KeyCode::S:
                        camera->keys.down = true;
                        break;
                    case KeyCode::A:
                        camera->keys.left = true;
                        break;
                    case KeyCode::D:
                        camera->keys.right = true;
                        break;
                    default:
                        break;
                }
                break;
            case KeyAction::Up:
                switch (code) {
                    case KeyCode::W:
                        camera->keys.up = false;
                        break;
                    case KeyCode::S:
                        camera->keys.down = false;
                        break;
                    case KeyCode::A:
                        camera->keys.left = false;
                        break;
                    case KeyCode::D:
                        camera->keys.right = false;
                        break;
                    default:
                        break;
                }
                break;
            case KeyAction::Repeat:
                break;
            case KeyAction::Unknown:
                break;
        }
    }
}

void Application::mainloop() {
    while (!glfwWindowShouldClose(window->getHandle())) {
        glfwPollEvents();
        update();
    }
}

