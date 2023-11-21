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


    // createPipeline();
    //  createDepthStencil();



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



void Application::createRenderContext()
{
    auto surface_priority_list = std::vector<VkSurfaceFormatKHR>{
        {VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
        {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}
    };
    renderContext = std::make_unique<RenderContext>(*device, surface, *window);
    RenderContext::g_context = renderContext.get();
}


void Application::initWindow(const char* name, int width, int height)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = std::make_unique<Window>(Window::WindowProp{"title"}, this);
}

void Application::initGUI()
{
    // gui = std::make_unique<Gui>(*device);
    // gui->prepare(pipelineCache, renderPipeline->getRenderPass().getHandle());
    // gui->prepareResoucrces(this);
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
