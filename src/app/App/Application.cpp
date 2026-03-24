//
// Created by 鎵撳伐浜?on 2023/3/19.
//

#include "Application.h"

#include "ctpl_stl.h"

#include <Core/Device/Instance.h>
#include "Common/VkCommon.h"
#include "Common/Config.h"
#include "Common/TextureHelper.h"
#include "Gui/Gui.h"
#include "Core/RenderTarget.h"
#include "Core/Shader/Shader.h"
#include "Core/Texture.h"
#include "Core/math.h"
#include "Core/Shader/GlslCompiler.h"
#include "IO/ImageIO.h"
#include "PostProcess/PostProcess.h"
#include "RenderPasses/RenderPassBase.h"
#include "Scene/Compoments/Camera.h"
#include "Scene/SceneLoader/ObjLoader.hpp"
#include "Scene/SceneLoader/SceneLoaderInterface.h"
#include "Scene/SceneLoader/gltfloader.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <imgui.h>
#include <imgui_internal.h>
#include "Core/SwapChain.h"
#include <volk.h>
#include <algorithm>

namespace {
const char* renderPassTypeToString(RenderPassType type) {
    switch (type) {
        case RenderPassType::GRAPHICS:
            return "Graphics";
        case RenderPassType::COMPUTE:
            return "Compute";
        case RenderPassType::RAYTRACING:
            return "RayTracing";
        case RenderPassType::TRANSFER:
            return "Transfer";
        default:
            return "Unknown";
    }
}

void beginEditorDockspace() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    constexpr ImGuiWindowFlags hostWindowFlags =
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoBackground;

    ImGui::Begin("EditorDockspace", nullptr, hostWindowFlags);
    ImGui::PopStyleVar(3);
    ImGuiID dockspaceId = ImGui::GetID("EditorDockspaceRoot");
    ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::End();
}

void setNextWindowDockSize(float width, float height, ImGuiCond cond = ImGuiCond_FirstUseEver) {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x * width, viewport->WorkSize.y * height), cond);
}
} // namespace


/*
 * Initializes window dimensions and application name
 * Calls initWindow to create the application window       
 */
Application::Application(const char* name,
                         uint32_t    width,
                         uint32_t    height,
                         RTConfing   _config) : mWidth(width), mHeight(height), mAppName(name), config(_config) {
    initWindow(name, width, height);
}
Application::Application(const char* name,  std::string configPath) :  mAppName(name), config(configPath) {
    mWidth = config.getWindowWidth();
    mHeight = config.getWindowHeight();
    initWindow(name, mWidth, mHeight);
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

/**
 * @brief init window
 */
void Application::initWindow(const char* name, uint32_t width, uint32_t height) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = std::make_unique<Window>(Window::WindowProp{name, {width, height}}, this);
}

/**
 * @brief Initialize Vulkan components:
 * - Initialize Vulkan loader
 * - Create Vulkan instance
 * - Create surface
 * - Select physical device
 * - Create logical device
 */
void Application::prepare() {

    initLogger();

    initVk();
    initGUI();

    TextureHelper::Initialize();
    RenderPtrManangr::Initalize();
    SamplerManager::Initialize(*device);
    
    camera = std::make_shared<Camera>();
    mPostProcessPass = std::make_unique<PostProcess>();
    mPostProcessPass->init();
}

/**
 * @brief Initialize Vulkan components:
 * - Initialize Vulkan loader
 * - Create Vulkan instance
 * - Create surface
 * - Select physical device
 * - Create logical device
 * - Create render context
 * - Create fence for synchronization
 */
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
    addDeviceExtension(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    addDeviceExtension( VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);

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

void Application::initGUI() {
    gui = std::make_unique<Gui>(*device);
    gui->prepareResoucrces(this);
}
void Application::initLogger() {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink    = std::make_shared<spdlog::sinks::basic_file_sink_mt>(mAppName, true);
    spdlog::logger logger(mAppName, {console_sink, file_sink});
    spdlog::set_default_logger(std::make_shared<spdlog::logger>(logger));
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

void Application::createRenderContext() {
    renderContext = std::make_unique<RenderContext>(*device, surface, *window);
    g_context     = renderContext.get();
}

void Application::mainloop() {
    while (!glfwWindowShouldClose(window->getHandle())) {
        glfwPollEvents();
        update();
    }
}

/**
 * @brief Updates the application state.
 * 
 * - Calculates the delta time since the last frame.
 * - Checks if the application is focused;
 * - Calls the onViewUpdated() method.
 * - Updates the scene and GUI.
 * - Begins a new frame in the render context.
 * - Waits for the fence to ensure synchronization.
 * - Resets the fence for the next frame.
 * ------------------------- Render Graph -------------------------
 * - Creates a render graph and imports the current hardware texture.
 * - Draws the frame and renders the post-process pass.
 * - Updates the list of current textures.
 * - Adds an image copy pass to the render graph.
 * - Handles saving the image if required.
 * - Adds the GUI pass to the render graph.
 * - Executes the render graph using the graphic command buffer.
 * ------------------------- Render Graph -------------------------
 * - Submits the command buffer and presents the frame.
 * - Resets the image save state.
 * - Updates the camera based on the delta time.
 */


void Application::update() {

    deltaTime = timer.tick<Timer::Seconds>();
    if (!m_focused)
        return;

    if (viewUpdated) {
        viewUpdated = false;
        onViewUpdated();
    }

    updateGUI();

    renderContext->beginFrame();

    vkWaitForFences(device->getHandle(), 1, &fence, VK_TRUE, UINT64_MAX);
    vkResetFences(device->getHandle(), 1, &fence);

    RenderGraph graph(*device);
    graph.setProfilingEnabled(mRenderGraphProfilingEnabled);
    graph.importTexture(RENDER_VIEW_PORT_IMAGE_NAME, &renderContext->getCurHwtexture());
    
    if (scene && scene->getLoadCompleteInfo().GetSceneLoaded()) {
        if(sceneFirstLoad){
            onSceneLoaded();
        }
        updateScene();
        drawFrame(graph);
        //mPostProcessPass->render(graph);
    }

    mCurrentTextures = graph.getResourceNames(RenderResourceType::ETexture);
    graph.addImageCopyPass(graph.getBlackBoard().getHandle(mPresentTexture), graph.getBlackBoard().getHandle(RENDER_VIEW_PORT_IMAGE_NAME));
    handleSaveImage(graph);

    gui->addGuiPass(graph);

    graph.execute(renderContext->getGraphicCommandBuffer());

    renderContext->submitAndPresent(renderContext->getGraphicCommandBuffer(), fence);

    mRenderGraphProfile.passes.clear();
    mRenderGraphProfile.totalCpuMs      = graph.getFrameProfile().totalCpuMs;
    mRenderGraphProfile.totalGpuMs      = 0.0;
    mRenderGraphProfile.activePassCount = graph.getFrameProfile().activePassCount;
    mRenderGraphProfile.gpuSupported    = renderContext->isTimestampProfilingSupported();
    mRenderGraphProfile.passes.reserve(graph.getFrameProfile().samples.size());
    for (const auto& sample : graph.getFrameProfile().samples) {
        mRenderGraphProfile.passes.push_back(RenderGraphProfileView::PassTiming{
            .name = sample.name,
            .type = renderPassTypeToString(sample.type),
            .gpuMs = 0.0,
            .cpuMs = sample.cpuMs,
        });
    }

    const auto& gpuProfile = renderContext->getResolvedTimestampProfile();
    mRenderGraphProfile.gpuSupported = gpuProfile.supported;
    mRenderGraphProfile.totalGpuMs   = gpuProfile.totalGpuMs;
    const size_t mergedCount = std::min(mRenderGraphProfile.passes.size(), gpuProfile.samples.size());
    for (size_t i = 0; i < mergedCount; ++i) {
        mRenderGraphProfile.passes[i].gpuMs = gpuProfile.samples[i].gpuMs;
    }

    if (mRenderGraphProfilingEnabled && mRenderGraphProfile.gpuSupported) {
        mRenderGraphFrameHistory.push_back(static_cast<float>(mRenderGraphProfile.totalGpuMs));
        if (mRenderGraphFrameHistory.size() > 180) {
            mRenderGraphFrameHistory.erase(mRenderGraphFrameHistory.begin());
        }
    }

    resetImageSave();

    camera->update(deltaTime);

    if (camera->moving()) {
        viewUpdated = true;
    }
}

void Application::updateScene() {
    if (sceneAsync != nullptr && sceneAsync->getLoadCompleteInfo().GetSceneLoaded() ) {
        scene = std::move(sceneAsync);
        onSceneLoaded();
        sceneAsync = nullptr;
    }
    if(view)
        view->perFrameUpdate();
}
/**
 * @brief Update GUI
 * 
 * 
 * Sets the display size and delta time for ImGui;
 * Starts a new frame;
 * Renders various GUI elements such as:
 * - frame time
 * - FPS
 * - file dialogs
 */
void Application::updateGUI() {
    if (!gui)
        return;

    ImGuiIO& io = ImGui::GetIO();

    io.DisplaySize = ImVec2(static_cast<float>(mWidth), static_cast<float>(mHeight));
    io.DeltaTime   = 0;

    gui->newFrame();
    beginEditorDockspace();

    setNextWindowDockSize(0.22f, 0.65f);
    if (ImGui::Begin("Workspace")) {
        ImGui::TextUnformatted("Session");
        ImGui::Separator();
        ImGui::Text("%.2f ms/frame", 1000.f * deltaTime);
        ImGui::Text("%u fps", deltaTime > 0.f ? toUint32(1.f / deltaTime) : 0);
        ImGui::Spacing();

        ImGui::Checkbox("Profile RenderGraph", &mRenderGraphProfilingEnabled);
        if (!renderContext->isTimestampProfilingSupported()) {
            ImGui::TextDisabled("GPU timestamps are not supported on this device/queue.");
        }
        ImGui::Checkbox("Save PNG", &imageSave.savePng);
        ImGui::Checkbox("Save EXR", &imageSave.saveExr);
        ImGui::Checkbox("Save Camera", &saveCamera);
        ImGui::Spacing();

        auto file = gui->showFileDialog("Open Scene", {".gltf", ".json"});
        if (file != "no file selected") {
            ctpl::thread_pool pool(1);
            pool.push([this, file](size_t) {
                LOGI("file selected: {}", file);
                sceneAsync = SceneLoaderInterface::LoadSceneFromFile(*device, file, sceneLoadingConfig);
            });
        }

        if (!mCurrentTextures.empty()) {
            auto itemIter = std::ranges::find(mCurrentTextures.begin(), mCurrentTextures.end(), mPresentTexture);
            int itemCurrent = itemIter != mCurrentTextures.end() ? static_cast<int>(itemIter - mCurrentTextures.begin()) : 0;
            std::vector<const char*> currentTexturesCStr;
            currentTexturesCStr.reserve(mCurrentTextures.size());
            std::ranges::transform(mCurrentTextures.begin(), mCurrentTextures.end(), std::back_inserter(currentTexturesCStr), [](const std::string& str) { return str.c_str(); });
            if (ImGui::Combo("Preview Target", &itemCurrent, currentTexturesCStr.data(), static_cast<int>(currentTexturesCStr.size()))) {
                mPresentTexture = mCurrentTextures[itemCurrent];
            } else if (itemCurrent >= 0 && itemCurrent < static_cast<int>(mCurrentTextures.size())) {
                mPresentTexture = mCurrentTextures[itemCurrent];
            }
        }

        if (scene) {
            ImGui::Spacing();
            ImGui::TextUnformatted("Scene");
            ImGui::Separator();
            ImGui::TextWrapped("%s", scene->getPath().string().c_str());
        }
    }
    ImGui::End();

    setNextWindowDockSize(0.25f, 0.80f);
    if (ImGui::Begin("Inspector")) {
        if (ImGui::CollapsingHeader("Renderer", ImGuiTreeNodeFlags_DefaultOpen)) {
            onUpdateGUI();
        }
        if (camera && ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
            camera->onShowInEditor();
        }
        if (ImGui::CollapsingHeader("Post Process", ImGuiTreeNodeFlags_DefaultOpen)) {
            mPostProcessPass->updateGui();
        }
        if (view && ImGui::CollapsingHeader("View", ImGuiTreeNodeFlags_DefaultOpen)) {
            view->updateGui();
        }
    }
    ImGui::End();

    setNextWindowDockSize(0.65f, 0.72f);
    auto& texture = g_context->getCurHwtexture();
    if (ImGui::Begin("Viewport")) {
        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        viewportSize.x = std::max(viewportSize.x, 1.0f);
        viewportSize.y = std::max(viewportSize.y, 1.0f);
        ImGui::Image(&texture.getVkImageView(), viewportSize, ImVec2(0, 0), ImVec2(1, 1));
    }
    ImGui::End();

    setNextWindowDockSize(0.65f, 0.28f);
    if (ImGui::Begin("RenderGraph")) {
        if (mRenderGraphProfile.gpuSupported) {
            ImGui::Text("Total GPU: %.3f ms", mRenderGraphProfile.totalGpuMs);
            ImGui::SameLine();
        }
        ImGui::Text("Total CPU: %.3f ms", mRenderGraphProfile.totalCpuMs);
        ImGui::SameLine();
        ImGui::Text("| Passes: %u", mRenderGraphProfile.activePassCount);

        if (!mRenderGraphFrameHistory.empty()) {
            float maxValue = 0.0f;
            for (float sample : mRenderGraphFrameHistory) {
                maxValue = std::max(maxValue, sample);
            }
            maxValue = std::max(maxValue, 0.1f);
            ImGui::PlotLines("Frame History", mRenderGraphFrameHistory.data(), static_cast<int>(mRenderGraphFrameHistory.size()), 0, nullptr, 0.0f, maxValue * 1.1f, ImVec2(0.0f, 72.0f));
        }

        if (ImGui::BeginTable("RenderGraphPassTable", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_ScrollY)) {
            ImGui::TableSetupColumn("Pass");
            ImGui::TableSetupColumn("Type");
            ImGui::TableSetupColumn("GPU ms", ImGuiTableColumnFlags_WidthFixed, 90.0f);
            ImGui::TableSetupColumn("CPU ms", ImGuiTableColumnFlags_WidthFixed, 90.0f);
            ImGui::TableHeadersRow();

            for (const auto& pass : mRenderGraphProfile.passes) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(pass.name.c_str());
                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(pass.type.c_str());
                ImGui::TableSetColumnIndex(2);
                if (mRenderGraphProfile.gpuSupported) {
                    ImGui::Text("%.3f", pass.gpuMs);
                } else {
                    ImGui::TextUnformatted("-");
                }
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%.3f", pass.cpuMs);
            }

            ImGui::EndTable();
        }
    }
    ImGui::End();

    ImGui::Render();
    ImGui::EndFrame();

    if (gui->update() || gui->updated) {
        gui->updated = false;
    }
}

void Application::resetImageSave() {
    if (imageSave.savePng | imageSave.saveExr) {

        auto width  = g_context->getViewPortExtent().width;
        auto height = g_context->getViewPortExtent().height;

        std::shared_ptr<std::vector<uint8_t>> data   = std::make_shared<std::vector<uint8_t>>(imageSave.buffer->getSize());
        auto                                  mapped = imageSave.buffer->map();
        memcpy(data->data(), mapped, data->size());
        imageSave.buffer->unmap();

        auto sceneFolderPath = scene->getPath().parent_path().string();
        auto imagePath = sceneFolderPath + "/output";
        ImageIO::saveImage(imagePath.c_str(), data, width, height, 4, imageSave.savePng, imageSave.saveExr);

        imageSave.saveExr = false;
        imageSave.savePng = false;
    }

    if (saveCamera) {
        Config::GetInstance().CameraToConfig(*camera,scene->getName());
        Config::GetInstance().SaveConfig();
        saveCamera = false;
    }
}

void Application::handleSaveImage(RenderGraph& graph) {
    if (imageSave.saveExr | imageSave.savePng) {
        graph.addComputePass(
            "image to file ",
            [&](RenderGraph::Builder& builder, ComputePassSettings& settings) {
                RenderGraphHandle image;
                if(imageSave.savePng)
                    image = graph.getBlackBoard().getHandle(getLdrImageToSave());
                else {
                     image = graph.getBlackBoard().getHandle(getHdrImageToSave());
                }
                builder.readTexture(image, TextureUsage::TRANSFER_SRC);

                //Not really write to image. Avoid pass cut
                builder.writeTexture(image, TextureUsage::TRANSFER_SRC);
            },
            [&](RenderPassContext& context) {
                auto& swapchainImage = graph.getBlackBoard().getHwImage(imageSave.savePng ? getLdrImageToSave() : getHdrImageToSave());
                auto  width          = swapchainImage.getExtent2D().width;
                auto  height         = swapchainImage.getExtent2D().height;
                auto  stride = imageSave.savePng ? 4 : 16;
                if (imageSave.buffer == nullptr || imageSave.buffer->getSize() < width * height * stride) {
                    imageSave.buffer = std::make_unique<Buffer>(*device, swapchainImage.getExtent2D().width * swapchainImage.getExtent2D().height * stride, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
                }
                VkBufferImageCopy region = {.bufferOffset      = 0,
                                            .bufferRowLength   = 0,
                                            .bufferImageHeight = 0,
                                            .imageSubresource  = {.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                                                                  .mipLevel       = 0,
                                                                  .baseArrayLayer = 0,
                                                                  .layerCount     = 1},
                                            .imageOffset       = {0, 0, 0},
                                            .imageExtent       = {width, height, 1}};
                vkCmdCopyImageToBuffer(context.commandBuffer.getHandle(), swapchainImage.getVkImage().getHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, imageSave.buffer->getHandle(), 1, &region);
            });
    }
}

void Application::setFocused(bool focused) {
    m_focused = focused;
}

// void Application::loadScene(const std::string& path,const SceneLoadingConfig & config)
// {
//     scene = GltfLoading::LoadSceneFromGLTFFile(*device,FileUtils::getResourcePath(path));
//     camera = scene->getCameras()[0];
// }

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
                    case KeyCode::Up:
                        camera->keys.up = true;
                        break;
                    case KeyCode::Down:
                        camera->keys.down = true;
                        break;
                    case KeyCode::Left:
                        camera->keys.left = true;
                        break;
                    case KeyCode::Right:
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
                    case KeyCode::Up:
                        camera->keys.up = false;
                        break;
                    case KeyCode::Down:
                        camera->keys.down = false;
                        break;
                    case KeyCode::Left:
                        camera->keys.left = false;
                        break;
                    case KeyCode::Right:
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

void Application::onResize(uint32_t width, uint32_t height) {
    this->mWidth  = width;
    this->mHeight = height;
}

void Application::loadScene(const std::string& path) {
    scene = SceneLoaderInterface::LoadSceneFromFile(*device, path, sceneLoadingConfig);
    //RuntimeSceneManager::addSponzaRestirLight(*scene);
}

void Application::onSceneLoaded() {
    if(!scene->getCameras().empty()) {
        camera = scene->getCameras()[0];
    }
    initView();
    Config::GetInstance().CameraFromConfig(*camera,scene->getName());
    sceneFirstLoad = false;

    
}

void Application::initView() {
    view = std::make_unique<View>(*device);
    if (scene->getCameras().empty()) {
        camera = std::make_shared<Camera>();
        camera->setPerspective(60.0f, (float)mWidth / (float)mHeight, 0.1f, 256.0f);
    }
    view->setScene(scene.get());
    view->setCamera(camera.get());

   // RenderPtrManangr::init();
    g_manager->putPtr("view", view.get());
}


void Application::handleMouseMove(float x, float y) {
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


