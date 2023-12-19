//
// Created by 打工人 on 2023/3/19.
//


#include "Application.h"
#include <Core/Device/Instance.h>
#include "../Common/VkCommon.h"
#include "Gui/Gui.h"
#include "Core/RenderTarget.h"
#include "Core/Shader/Shader.h"
#include "Core/Texture.h"
#include "Scene/Compoments/Camera.h"
#include "Scene/SceneLoader/gltfloader.h"


void Application::initVk() {
    VK_CHECK_RESULT(volkInitialize());

    getRequiredInstanceExtensions();
    _instance = std::make_unique<Instance>(std::string("vulkanApp"), instanceExtensions, validationLayers);
    surface = window->createSurface(*_instance);

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

    device = std::make_unique<Device>(physical_devices[0], surface,_instance->getHandle(), deviceExtensions);

    createRenderContext();

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
}

void Application::updateScene() {
}

void Application::updateGUI() {
    if (!gui)
        return;
    ImGuiIO &io = ImGui::GetIO();

    io.DisplaySize = ImVec2((float) 1024, (float) 1024);
    io.DeltaTime = 0;


    ImGui::NewFrame();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
    ImGui::SetNextWindowPos(ImVec2(10 * gui->scale, 10 * gui->scale));
    ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
    ImGui::Begin("Vulkan Example", nullptr,
                 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::PushItemWidth(200.0f * gui->scale);
    ImGui::Text("%.2f ms/frame ", 1000.f * deltaTime);
    ImGui::NextColumn();
    ImGui::Text(" %d fps",toUint32(1.f/deltaTime));
    ImGui::InputFloat3("Camera Position", &camera->position[0], 2);
    ImGui::InputFloat3("Camera Rotation", &camera->rotation[0], 2);
    ImGui::PopItemWidth(); 
    onUpdateGUI();
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::Render();

    if (gui->update() || gui->updated) {
        gui->updated = false;
    }
}



void Application::update() {
    deltaTime = timer.tick<Timer::Seconds>();

    if (viewUpdated) {
        viewUpdated = false;
        onViewUpdated();
    }

    updateScene();
    updateGUI();


    renderContext->beginFrame();
    
    vkWaitForFences(device->getHandle(), 1, &fence, VK_TRUE, UINT64_MAX);
    vkResetFences(device->getHandle(), 1, &fence);

    RenderGraph graph(*device);
    auto handle = graph.importTexture(SWAPCHAIN_IMAGE_NAME,&renderContext->getCurHwtexture());
    graph.getBlackBoard().put(SWAPCHAIN_IMAGE_NAME,handle);
    
    drawFrame(graph);
    
    renderContext->submitAndPresent(renderContext->getGraphicCommandBuffer(),fence);
    
    camera->update(deltaTime);

    if (camera->moving()) {
        viewUpdated = true;
    }
}


void Application::createRenderContext() {
    renderContext = std::make_unique<RenderContext>(*device, surface, *window);
    RenderContext::g_context = renderContext.get();
}

// void Application::loadScene(const std::string& path,const SceneLoadingConfig & config)
// {
//     scene = GltfLoading::LoadSceneFromGLTFFile(*device,FileUtils::getResourcePath(path));
//     camera = scene->getCameras()[0];
// }


void Application::initWindow(const char *name, uint32_t width, uint32_t height) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = std::make_unique<Window>(Window::WindowProp{"title",{width,height}}, this);
}

void Application::initGUI() {
    gui = std::make_unique<Gui>(*device);
    gui->prepareResoucrces(this);
}


void Application::prepare() {
    initVk();
    initGUI();

}

Application::Application(const char *name,  uint32_t width, uint32_t height) : width(width), height(height) {
    initWindow(name, width, height);
}


void Application::inputEvent(const InputEvent &inputEvent)
{
    if(gui)
    {
        if(gui->inputEvent(inputEvent))
            return;
    }
    
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
    if (source == EventSource::Mouse) {
        const auto &mouseEvent = static_cast<const MouseButtonInputEvent &>(inputEvent);
        handleMouseMove(mouseEvent.getPosX(), mouseEvent.getPosY());
        auto action = mouseEvent.getAction();
        auto button = mouseEvent.getButton();
        switch (action) {
            case MouseAction::Down:
                switch (button) {
                    case MouseButton::Left:
                        camera->mouseButtons.left = true;
                        break;
                    case MouseButton::Right:
                        camera->mouseButtons.right = true;
                        break;
                    case MouseButton::Middle:
                        camera->mouseButtons.middle = true;
                        break;
                    default:
                        break;
                }
                break;
            case MouseAction::Up:
                switch (button) {
                    case MouseButton::Left:
                        camera->mouseButtons.left = false;
                        break;
                    case MouseButton::Right:
                        camera->mouseButtons.right = false;
                        break;
                    case MouseButton::Middle:
                        camera->mouseButtons.middle = false;
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
    } else if (source == EventSource::TouchScreen) {
        const auto &touchEvent = static_cast<const TouchInputEvent &>(inputEvent);

        if (touchEvent.getAction() == TouchAction::Down) {
            //   touchDown = true;
            touchPos.x = static_cast<int32_t>(touchEvent.getPosX());
            touchPos.y = static_cast<int32_t>(touchEvent.getPosY());
            mousePos.x = touchEvent.getPosX();
            mousePos.y = touchEvent.getPosY();
            camera->mouseButtons.left = true;
        } else if (touchEvent.getAction() == TouchAction::Up) {
            touchPos.x = static_cast<int32_t>(touchEvent.getPosX());
            touchPos.y = static_cast<int32_t>(touchEvent.getPosY());
            //   touchTimer = 0.0;
            //   touchDown = false;
            camera->keys.up = false;
            camera->mouseButtons.left = false;
        } else if (touchEvent.getAction() == TouchAction::Move) {
            bool handled = false;
            if (gui) {
                ImGuiIO &io = ImGui::GetIO();
                handled = io.WantCaptureMouse;
            }
            if (!handled) {
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

void Application::mainloop() {
    while (!glfwWindowShouldClose(window->getHandle())) {
        glfwPollEvents();
        update();
    }
}

Application::~Application()
{
    scene.reset();
    camera.reset();
    renderContext.reset();
    gui.reset();
    device.reset();
    _instance.reset();
    window.reset();
}

void Application::handleMouseMove(float x, float y) {
    bool handled = false;
    float dx = static_cast<int32_t>(mousePos.x) - x;
    float dy = static_cast<int32_t>(mousePos.y) - y;
    onMouseMove();


    if (camera->mouseButtons.left) {
        rotation.x += dy * 1.25f * rotationSpeed;
        rotation.y -= dx * 1.25f * rotationSpeed;
        camera->rotate(glm::vec3(dy * camera->rotationSpeed, -dx * camera->rotationSpeed, 0.0f));
        viewUpdated = true;
    }
    if (camera->mouseButtons.right) {
        camera->translate(glm::vec3(-0.0f, 0.0f, dy * .005f));
        viewUpdated = true;
    }
    if (camera->mouseButtons.middle) {
        camera->translate(glm::vec3(-dx * 0.01f, -dy * 0.01f, 0.0f));
        viewUpdated = true;
    }
    mousePos = glm::vec2(static_cast<float>(x), static_cast<float>(y));
}

void Application::onUpdateGUI() {
    
}

void Application::onMouseMove() {
}

void Application::onViewUpdated() {
}
