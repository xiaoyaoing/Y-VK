//
// Created by 打工人 on 2023/3/19.
//
#include "Application.h"
#include "Instance.h"
#include "Scene.h"
#include "../Common/VkCommon.h"
#include "Gui.h"
#include <RenderTarget.h>
#include <Shader.h>
#include <Subpass.h>
#include <API_VK.h>
#include <Camera.h>


void Application::initVk()
{
    getRequiredInstanceExtensions();
    _instance = std::make_unique<Instance>(std::string("vulkanApp"), instanceExtensions, validationLayers);
    surface = window->createSurface(*_instance);

    // create device
    uint32_t physical_device_count{0};
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(_instance->getHandle(), &physical_device_count, nullptr));

    if (physical_device_count < 1)
    {
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
    //createPipelineCache();
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

void Application::getRequiredInstanceExtensions()
{
    uint32_t glfwExtensionsCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);
    for (uint32_t i = 0; i < glfwExtensionsCount; i++)
    {
        addInstanceExtension(glfwExtensions[i]);
    }
    if (enableValidationLayers)
        addInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    //  addInstanceExtension(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
}

void Application::updateScene()
{
}

void Application::updateGUI()
{
    if (!gui)
        return;
    ImGuiIO& io = ImGui::GetIO();

    io.DisplaySize = ImVec2((float)1024, (float)1024);
    io.DeltaTime = 0;
    //
    //    io.MousePos = ImVec2(mousePos.x, mousePos.y);
    //    io.MouseDown[0] = mouseButtons.left && UIOverlay.visible;
    //    io.MouseDown[1] = mouseButtons.right && UIOverlay.visible;
    //    io.MouseDown[2] = mouseButtons.middle && UIOverlay.visible;

    ImGui::NewFrame();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
    ImGui::SetNextWindowPos(ImVec2(10 * gui->scale, 10 * gui->scale));
    ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
    ImGui::Begin("Vulkan Example", nullptr,
                 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::TextUnformatted("Vk Example");
    ImGui::TextUnformatted("Device");
    ImGui::Text("%.2f ms/frame (%.1d fps)", (1000.0f / 1, 1));


    ImGui::PushItemWidth(110.0f * gui->scale);
    // OnUpdateUIOverlay(&UIOverlay);
    ImGui::PopItemWidth();
    onUpdateGUI();
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::Render();

    if (gui->update() || gui->updated)
    {
        buildCommandBuffers();
        gui->updated = false;
    }
}

void Application::createFrameBuffers()
{
}

void Application::createCommandBuffer()
{
    //    commandPool = std::make_unique<CommandPool>(*device,
    //                                                device->getQueueByFlag(VK_QUEUE_GRAPHICS_BIT, 0).getFamilyIndex(),
    //                                                CommandBuffer::ResetMode::AlwaysAllocate);

    auto frameCount = renderContext->getSwapChainImageCount();
    commandBuffers.reserve(frameCount);
    VkCommandBufferAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = device->getCommandPool().getHandle();
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = frameCount;
    std::vector<VkCommandBuffer> vkCommandBuffers(frameCount);

    VK_CHECK_RESULT(vkAllocateCommandBuffers(device->getHandle(), &allocateInfo, vkCommandBuffers.data()))

    for (const auto& vkCommandBuffer : vkCommandBuffers)
        commandBuffers.emplace_back(std::move(std::make_unique<CommandBuffer>(vkCommandBuffer)));
}

void Application::createRenderPass()
{
    // // //use RenderTarget Structure
    // // assert(renderContext->isPrepared() && "RenderContext must be initialized");
    // // auto &renderTarget = renderContext->getRenderFrame(0).getRenderTarget();
    // // renderPipeline->createRenderPass(renderTarget, loadStoreInfos);
    // //
    // // return;
    //
    // VkAttachmentDescription colorAttachment{};
    // colorAttachment.format = renderContext->getSwapChainFormat();
    // colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    //
    // colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    // colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    // colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    // colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    // VkAttachmentDescription depthAttachment{};
    //
    // depthAttachment.format = VK_FORMAT_D32_SFLOAT;
    // depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    // depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    // depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    // depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    //
    // VkAttachmentReference colorAttachmentRef{};
    // colorAttachmentRef.attachment = 0;
    // colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    //
    // VkAttachmentReference depthAttachmentRef{};
    // depthAttachmentRef.attachment = 1;
    // depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    //
    // VkSubpassDescription subpass{};
    // subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    // subpass.colorAttachmentCount = 1;
    // subpass.pColorAttachments = &colorAttachmentRef;
    // subpass.pDepthStencilAttachment = &depthAttachmentRef;
    // subpass.inputAttachmentCount = 0;
    // subpass.pInputAttachments = nullptr;
    // subpass.preserveAttachmentCount = 0;
    // subpass.pPreserveAttachments = nullptr;
    // subpass.pResolveAttachments = nullptr;
    //
    // std::array<VkSubpassDependency, 2> dependencies;
    //
    // dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    // dependencies[0].dstSubpass = 0;
    // dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    // dependencies[0].dstStageMask =
    //         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
    //         VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    // dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    // dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
    //                                 VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
    //                                 VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    // dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    //
    // dependencies[1].srcSubpass = 0;
    // dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    // dependencies[1].srcStageMask =
    //         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
    //         VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    // dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    // dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
    //                                 VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
    //                                 VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    // dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    // dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    //
    // std::vector<VkAttachmentDescription> attachments = {colorAttachment, depthAttachment};
    // std::vector<VkSubpassDescription> subpasses = {subpass};
    //
    // VkRenderPassCreateInfo render_pass_create_info = {};
    // render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    // render_pass_create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
    // render_pass_create_info.pAttachments = attachments.data();
    // render_pass_create_info.subpassCount = 1;
    // render_pass_create_info.pSubpasses = &subpass;
    // render_pass_create_info.dependencyCount = static_cast<uint32_t>(dependencies.size());
    // render_pass_create_info.pDependencies = dependencies.data();
    //
    // VkRenderPass vkRenderPass;
    // VK_CHECK_RESULT(vkCreateRenderPass(device->getHandle(), &render_pass_create_info, nullptr, &vkRenderPass));
    //
    // renderPipeline->createRenderPass(vkRenderPass);
}

void Application::createDepthStencil()
{
    //    auto depthImageInfo = Image::getDefaultImageInfo();
    //    depthImageInfo.extent = VkExtent3D{_context->getSwapChainExtent().width, _context->getSwapChainExtent().height, 1};
    //    depthImageInfo.format = VK_FORMAT_D32_SFLOAT;
    //    depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    //    depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
    //    depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    //    _depthImage = std::make_shared<Image>(_allocator, VMA_MEMORY_USAGE_GPU_ONLY, depthImageInfo);
    //    _depthImageView = std::make_shared<ImageView>(device, _depthImage, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

void Application::createAllocator()
{
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = device->getPhysicalDevice();
    allocatorInfo.device = device->getHandle();
    allocatorInfo.instance = _instance->getHandle();

    VK_CHECK_RESULT(vmaCreateAllocator(&allocatorInfo, &_allocator));
    device->setMemoryAllocator(_allocator);
}

void Application::update()
{
    auto tStart = std::chrono::high_resolution_clock::now();

    if (viewUpdated)
    {
        viewUpdated = false;
        onViewUpdated();
    }

    updateScene();
    updateGUI();

    drawFrame();

    auto tEnd = std::chrono::high_resolution_clock::now();

    auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();

    frameTimer = tDiff / 1000.f;
    camera->update(frameTimer);

    if (camera->moving())
    {
        viewUpdated = true;
    }
}

// HWND Application::setupWindow(HINSTANCE hinstance, WNDPROC wndproc)
// {
//     return HWND();
// }

// void Application::handleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
// {
// }

void Application::draw(CommandBuffer& commandBuffer)
{
    bindUniformBuffers(commandBuffer);
    //   renderPipeline->draw(commandBuffer, renderFrame);

    const VkViewport viewport = vkCommon::initializers::viewport((float)width, (float)height, 0.0f, 1.0f);
    const VkRect2D scissor = vkCommon::initializers::rect2D(width, height, 0, 0);
    vkCmdSetViewport(commandBuffer.getHandle(), 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer.getHandle(), 0, 1, &scissor);
    gui->draw(commandBuffer.getHandle());

    commandBuffer.endRenderPass();

    {
        ImageMemoryBarrier memory_barrier{};
        memory_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        memory_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        memory_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        memory_barrier.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        memory_barrier.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

        // commandBuffer.imageMemoryBarrier(renderFrame.getRenderTarget().getViews()[0], memory_barrier);
    }
}

void Application::createRenderContext()
{
    auto surface_priority_list = std::vector<VkSurfaceFormatKHR>{
        {VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
        {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}
    };
    renderContext = std::make_unique<RenderContext>(*device, surface, *window);
    RenderContext::g_context = renderContext.get();
}

void Application::drawFrame()
{
    vkWaitForFences(device->getHandle(), 1, &fence, VK_TRUE, UINT64_MAX);
    vkResetFences(device->getHandle(), 1, &fence);
    renderContext->beginFrame();
    auto& commandBuffer = *commandBuffers[renderContext->getActiveFrameIndex()];
    renderContext->submit(commandBuffer, fence);
}

void Application::drawRenderPasses(CommandBuffer& buffer, RenderTarget& renderTarget)
{
    //    renderPipeline->draw(buffer, renderTarget);
}

void Application::initWindow(const char* name, int width, int height)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = std::make_unique<Window>(Window::WindowProp{"title"}, this);
    //    glfwSetFramebufferSizeCallback(window->getHandle(), [](GLFWwindow *window, int width, int height) {
    //        auto *app = reinterpret_cast<Application *>(glfwGetWindowUserPointer(window));
    //        app->frameBufferResized = true;
    //    });
}

void Application::initGUI()
{
    // gui = std::make_unique<Gui>(*device);
    // gui->prepare(pipelineCache, renderPipeline->getRenderPass().getHandle());
    // gui->prepareResoucrces(this);
}

void Application::createCommandPool()
{
}

VkPhysicalDevice Application::createPhysicalDevice()
{
    return nullptr;
}

void Application::prepare()
{
    initVk();
    initGUI();
}

Application::Application(const char* name, int width, int height) : width(width), height(height)
{
    initWindow(name, width, height);
}

void Application::createRenderPipeline()
{
    scene = std::make_unique<Scene>(*device);
    renderPipeline = std::make_unique<RenderPipeline>(*device);
    renderPipeline->addSubPass(std::make_unique<GeomSubpass>(*scene));
    std::vector<LoadStoreInfo> infos;
    infos.emplace_back(LoadStoreInfo{VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE});
    infos.emplace_back(LoadStoreInfo{VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE});
}


void Application::inputEvent(const InputEvent& inputEvent)
{
    auto source = inputEvent.getSource();

    if (source == EventSource::KeyBoard)
    {
        const auto& keyEvent = static_cast<const KeyInputEvent&>(inputEvent);
        auto action = keyEvent.getAction();
        auto code = keyEvent.getCode();

        switch (action)
        {
        case KeyAction::Down:
            switch (code)
            {
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
            switch (code)
            {
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
    if (source == EventSource::Mouse)
    {
        const auto& mouseEvent = static_cast<const MouseButtonInputEvent&>(inputEvent);
        handleMouseMove(mouseEvent.getPosX(), mouseEvent.getPosY());
        auto action = mouseEvent.getAction();
        auto button = mouseEvent.getButton();
        switch (action)
        {
        case MouseAction::Down:
            switch (button)
            {
            case MouseButton::Left:
                mouseButtons.left = true;
                break;
            case MouseButton::Right:
                mouseButtons.right = true;
                break;
            case MouseButton::Middle:
                mouseButtons.middle = true;
                break;
            default:
                break;
            }
            break;
        case MouseAction::Up:
            switch (button)
            {
            case MouseButton::Left:
                mouseButtons.left = false;
                break;
            case MouseButton::Right:
                mouseButtons.right = false;
                break;
            case MouseButton::Middle:
                mouseButtons.middle = false;
                break;
            default:
                break;
            }
            break;
        case MouseAction::Move:
            break;
        case MouseAction::Unknown:
            break;
        }
    }
    else if (source == EventSource::TouchScreen)
    {
        const auto& touchEvent = static_cast<const TouchInputEvent&>(inputEvent);

        if (touchEvent.getAction() == TouchAction::Down)
        {
            //   touchDown = true;
            touchPos.x = static_cast<int32_t>(touchEvent.getPosX());
            touchPos.y = static_cast<int32_t>(touchEvent.getPosY());
            mousePos.x = touchEvent.getPosX();
            mousePos.y = touchEvent.getPosY();
            mouseButtons.left = true;
        }
        else if (touchEvent.getAction() == TouchAction::Up)
        {
            touchPos.x = static_cast<int32_t>(touchEvent.getPosX());
            touchPos.y = static_cast<int32_t>(touchEvent.getPosY());
            //   touchTimer = 0.0;
            //   touchDown = false;
            camera->keys.up = false;
            mouseButtons.left = false;
        }
        else if (touchEvent.getAction() == TouchAction::Move)
        {
            bool handled = false;
            if (gui)
            {
                ImGuiIO& io = ImGui::GetIO();
                handled = io.WantCaptureMouse;
            }
            if (!handled)
            {
                int32_t eventX = static_cast<int32_t>(touchEvent.getPosX());
                int32_t eventY = static_cast<int32_t>(touchEvent.getPosY());

                float deltaX = static_cast<float>(touchPos.y - eventY) * rotationSpeed * 0.5f;
                float deltaY = static_cast<float>(touchPos.x - eventX) * rotationSpeed * 0.5f;

                camera->rotate(glm::vec3(deltaX, 0.0f, 0.0f));
                camera->rotate(glm::vec3(0.0f, -deltaY, 0.0f));

                rotation.x += deltaX;
                rotation.y -= deltaY;

                // viewChanged();

                touchPos.x = eventX;
                touchPos.y = eventY;
            }
        }
    }
}

void Application::mainloop()
{
    while (!glfwWindowShouldClose(window->getHandle()))
    {
        glfwPollEvents();
        update();
    }
}

void Application::handleMouseMove(float x, float y)
{
    bool handled = false;
    float dx = static_cast<int32_t>(mousePos.x) - x;
    float dy = static_cast<int32_t>(mousePos.y) - y;
    onMouseMove();


    if (mouseButtons.left)
    {
        rotation.x += dy * 1.25f * rotationSpeed;
        rotation.y -= dx * 1.25f * rotationSpeed;
        camera->rotate(glm::vec3(dy * camera->rotationSpeed, -dx * camera->rotationSpeed, 0.0f));
        viewUpdated = true;
    }
    if (mouseButtons.right)
    {
        //   zoom += dy * .005f * zoom_speed;
        camera->translate(glm::vec3(-0.0f, 0.0f, dy * .005f));
        viewUpdated = true;
    }
    if (mouseButtons.middle)
    {
        //   camera_pos.x -= dx * 0.01f;
        //     camera_pos.y -= dy * 0.01f;
        camera->translate(glm::vec3(-dx * 0.01f, -dy * 0.01f, 0.0f));
        viewUpdated = true;
    }
    mousePos = glm::vec2(static_cast<float>(x), static_cast<float>(y));
}

void Application::onUpdateGUI()
{
}

void Application::onMouseMove()
{
}

void Application::onViewUpdated()
{
}
