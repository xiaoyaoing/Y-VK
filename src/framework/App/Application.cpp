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
#include "Core/math.h"
#include "Core/Shader/GlslCompiler.h"
#include "IO/ImageIO.h"
#include "RenderPasses/RenderPassBase.h"
#include "Scene/Compoments/Camera.h"
#include "Scene/SceneLoader/gltfloader.h"

void Application::initVk() {
    VK_CHECK_RESULT(volkInitialize());

    getRequiredInstanceExtensions();
    _instance = std::make_unique<Instance>(std::string("vulkanApp"), instanceExtensions, validationLayers);
    surface   = window->createSurface(*_instance);

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
    addDeviceExtension(VK_KHR_MAINTENANCE1_EXTENSION_NAME);

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physical_devices[0], &deviceProperties);
    LOGI("Device Name: {}", deviceProperties.deviceName)

    device = std::make_unique<Device>(physical_devices[0], surface, _instance->getHandle(), deviceExtensions);

    createRenderContext();

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    VK_CHECK_RESULT(vkCreateFence(device->getHandle(), &fenceInfo, nullptr, &fence));
}

void Application::getRequiredInstanceExtensions() {
    uint32_t     glfwExtensionsCount = 0;
    const char** glfwExtensions      = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);
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


    ImGuiIO& io = ImGui::GetIO();

    io.DisplaySize = ImVec2(static_cast<float>(mWidth), static_cast<float>(mHeight));
    io.DeltaTime   = 0;

    gui->newFrame();

    // static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
    //
				// // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
				// // because it would be confusing to have two docking targets within each others.
				// ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
    //
				// const ImGuiViewport* viewport = ImGui::GetMainViewport();
				// ImGui::SetNextWindowPos(viewport->WorkPos);
				// ImGui::SetNextWindowSize(viewport->WorkSize);
				// ImGui::SetNextWindowViewport(viewport->ID);
				// ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
				// ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
				// window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
				// window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    //
				// // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
				// // and handle the pass-thru hole, so we ask Begin() to not render a background.
				// if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
				// 	window_flags |= ImGuiWindowFlags_NoBackground;
    //
				// // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
				// // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
				// // all active windows docked into it will lose their parent and become undocked.
				// // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
				// // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
				// ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
				// ImGui::Begin("DockSpace Demo", nullptr, window_flags);
				// ImGui::PopStyleVar();
    //
				// ImGui::PopStyleVar(2);
    //
				// // Submit the DockSpace
				// if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
				// {
				// 	ImGuiID dockspace_id = ImGui::GetID("VulkanAppDockspace");
				// 	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
				// }

    // auto extent = renderContext->getSwapChainExtent();  
    // ImGui::SetNextWindowSize(ImVec2(extent.width,extent.height), ImGuiCond_FirstUseEver);
    // ImGui::Begin("Y-Vulkan", nullptr, ImGuiWindowFlags_MenuBar);

    // ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{10.f,10.f});
    // ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,      ImVec2{1.0f, 1.0f});
    // ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2{3.0f, 3.0f});
    //
    // ImGui::SetNextWindowPos(ImVec2(0,0));
    ImGui::Begin("Basic",nullptr,ImGuiWindowFlags_NoMove);
    // ImGui::SetWindowSize(ImVec2(300,0));
    //ImGui::PushItemWidth(200.0f * gui->scale);
    ImGui::Text("%.2f ms/frame ", 1000.f * deltaTime);
    ImGui::NextColumn();
    ImGui::Text(" %d fps", toUint32(1.f / deltaTime));
    ImGui::Checkbox("save png", &imageSave.savePng);
    ImGui::Checkbox("save exr", &imageSave.saveExr);

    auto itemIter    = std::ranges::find(mCurrentTextures.begin(), mCurrentTextures.end(), mPresentTexture);
    int  itemCurrent = itemIter - mCurrentTextures.begin();
    std::vector<const char*> currentTexturesCStr;
    std::ranges::transform(mCurrentTextures.begin(), mCurrentTextures.end(), std::back_inserter(currentTexturesCStr), [](const std::string& str) { return str.c_str(); });
    ImGui::Combo("RenderGraphTextures", &itemCurrent, currentTexturesCStr.data(), currentTexturesCStr.size());
    mPresentTexture = mCurrentTextures[itemCurrent];
    ImGui::End();
    ImGui::Separator();


    ImGui::Begin("camera",nullptr,ImGuiWindowFlags_NoMove);
    camera->onShowInEditor();
    ImGui::End();
    ImGui::Separator();

    
    ImGui::Begin("app sepcify",nullptr,ImGuiWindowFlags_NoMove);
    onUpdateGUI();
    ImGui::End();
    ImGui::Separator();

    
    auto & texture = g_context->getCurHwtexture();
    
  //  ImGui::PopStyleVar();
  //  ImGui::End();
    ImGui::SameLine();

    ImGui::Begin("Render view port",nullptr,ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoMouseInputs); // Leave room for 1 line below us
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,      ImVec2{0.f,0.f});

    ImVec2 size;
    auto p = ImGui::GetWindowPos();
    size.x = ImGui::GetWindowWidth();
    size.y = ImGui::GetWindowHeight();
    
    ImGui::Image(&texture.getVkImageView(),size, ImVec2(0, 0), ImVec2(1, 1));
    ImGui::PopStyleVar();
    ImGui::End();

    // ImGui::End();

  //  ImGui::End();

    ImGui::Render();
    ImGui::EndFrame();

    if (gui->update() || gui->updated) {
        gui->updated = false;
    }
}

void Application::update() {

    deltaTime = timer.tick<Timer::Seconds>();
    if (!m_focused)
        return;

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
    auto        handle = graph.importTexture(RENDER_VIEW_PORT_IMAGE_NAME, &renderContext->getCurHwtexture());
    graph.getBlackBoard().put(RENDER_VIEW_PORT_IMAGE_NAME, handle);

    drawFrame(graph);

    mCurrentTextures = graph.getResourceNames(RENDER_GRAPH_RESOURCE_TYPE::ETexture);
    graph.addImageCopyPass(graph.getBlackBoard().getHandle(mPresentTexture), graph.getBlackBoard().getHandle(RENDER_VIEW_PORT_IMAGE_NAME));

    handleSaveImage();
    
    gui->addGuiPass(graph);

    graph.execute(renderContext->getGraphicCommandBuffer());

    renderContext->submitAndPresent(renderContext->getGraphicCommandBuffer(), fence);

    resetImageSave();

    camera->update(deltaTime);

    if (camera->moving()) {
        viewUpdated = true;
    }
}

void Application::createRenderContext() {
    renderContext = std::make_unique<RenderContext>(*device, surface, *window);
    g_context     = renderContext.get();
}
void Application::resetImageSave() {
    if (imageSave.savePng | imageSave.saveExr) {

        auto width  = g_context->getViewPortExtent().width;
        auto height = g_context->getViewPortExtent().height;

        std::shared_ptr<std::vector<uint8_t>> data   = std::make_shared<std::vector<uint8_t>>(width * height * 4);
        auto                                  mapped = imageSave.buffer->map();
        memcpy(data->data(), mapped, data->size());
        imageSave.buffer->unmap();
        ImageIO::saveImage("output.png", data, width, height, 4, imageSave.savePng, imageSave.saveExr);

        imageSave.saveExr = false;
        imageSave.savePng = false;
    }
}

void Application::handleSaveImage() {
}


void Application::setFocused(bool focused) {
    //m_focused = focused;
}

// void Application::loadScene(const std::string& path,const SceneLoadingConfig & config)
// {
//     scene = GltfLoading::LoadSceneFromGLTFFile(*device,FileUtils::getResourcePath(path));
//     camera = scene->getCameras()[0];
// }

void Application::initWindow(const char* name, uint32_t width, uint32_t height) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = std::make_unique<Window>(Window::WindowProp{name, {width, height}}, this);
}

void Application::initGUI() {
    gui = std::make_unique<Gui>(*device);
    gui->prepareResoucrces(this);
}

void Application::prepare() {
    initVk();
    initGUI();
}

Application::Application(const char* name, uint32_t width, uint32_t height) : mWidth(width), mHeight(height),mAppName(name) {
    initWindow(name, width, height);
}

void Application::inputEvent(const InputEvent& inputEvent) {
    if (gui) {
        if (gui->inputEvent(inputEvent))
            return;
    }

    auto source = inputEvent.getSource();
    if (source == EventSource::KeyBoard) {
        const auto& keyEvent = static_cast<const KeyInputEvent&>(inputEvent);
        auto        action   = keyEvent.getAction();
        auto        code     = keyEvent.getCode();

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
        const auto& mouseEvent = static_cast<const MouseButtonInputEvent&>(inputEvent);
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
        const auto& touchEvent = static_cast<const TouchInputEvent&>(inputEvent);

        if (touchEvent.getAction() == TouchAction::Down) {
            //   touchDown = true;
            touchPos.x                = static_cast<int32_t>(touchEvent.getPosX());
            touchPos.y                = static_cast<int32_t>(touchEvent.getPosY());
            mousePos.x                = touchEvent.getPosX();
            mousePos.y                = touchEvent.getPosY();
            camera->mouseButtons.left = true;
        } else if (touchEvent.getAction() == TouchAction::Up) {
            touchPos.x = static_cast<int32_t>(touchEvent.getPosX());
            touchPos.y = static_cast<int32_t>(touchEvent.getPosY());
            //   touchTimer = 0.0;
            //   touchDown = false;
            camera->keys.up           = false;
            camera->mouseButtons.left = false;
        } else if (touchEvent.getAction() == TouchAction::Move) {
            bool handled = false;
            if (gui) {
                ImGuiIO& io = ImGui::GetIO();
                handled     = io.WantCaptureMouse;
            }
            if (!handled) {
                int32_t eventX = static_cast<int32_t>(touchEvent.getPosX());
                int32_t eventY = static_cast<int32_t>(touchEvent.getPosY());

                float deltaX = static_cast<float>(touchPos.y - eventY) * rotationSpeed * 0.5f;
                float deltaY = static_cast<float>(touchPos.x - eventX) * rotationSpeed * 0.5f;

                camera->pitch(math::toRadians(deltaX));
                camera->rotateY(math::toRadians(deltaY));
                // camera->rotate(glm::vec3(deltaX, 0.0f, 0.0f));
                // camera->rotate(glm::vec3(0.0f, -deltaY, 0.0f));

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
void Application::onResize(uint32_t width, uint32_t height) {
    this->mWidth  = width;
    this->mHeight = height;
}
void Application::initView() {
    view = std::make_unique<View>(*device);
    if (scene->getCameras().empty()) {
        camera = std::make_shared<Camera>();
        camera->setPerspective(60.0f, (float)mWidth / (float)mHeight, 0.1f, 256.0f);
    }
    view->setScene(scene.get());
    view->setCamera(camera.get());

    RenderPtrManangr::init();
    g_manager->putPtr("view", view.get());
}

Application::~Application() {
    scene.reset();
    camera.reset();
    renderContext.reset();
    gui.reset();
    device.reset();
    _instance.reset();
    window.reset();
}

void Application::handleMouseMove(float x, float y) {
    bool  handled = false;
    float dx      = static_cast<int32_t>(mousePos.x) - x;
    float dy      = static_cast<int32_t>(mousePos.y) - y;
    onMouseMove();

    if (camera->mouseButtons.right) {
        rotation.x += dy * 1.25f * rotationSpeed;
        rotation.y -= dx * 1.25f * rotationSpeed;
        //camera->rotate(glm::vec3(dy * camera->rotationSpeed, -dx * camera->rotationSpeed, 0.0f));
        camera->pitch(math::toRadians(dy * 0.1f));
        camera->rotateY(math::toRadians(0.1f * dx));
        viewUpdated = true;
    }
    if (camera->mouseButtons.left) {
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
void Application::preparePerViewData() {
}