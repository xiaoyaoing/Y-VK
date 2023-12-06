#pragma once

#include "Core/Buffer.h"
#include "Core/CommandBuffer.h"
#include "Core/Pipeline.h"
#include "Core/FrameBuffer.h"
#include "Core/RenderPass.h"
#include "Core/Device/Device.h"
#include <Core/Images/Image.h>
#include "Core/Queue.h"
#include "PlatForm/Window.h"
#include "Core/CommandPool.h"
#include "Core/SwapChain.h"
#include "Core/RenderTarget.h"
#include "Core/RenderContext.h"
#include <Core/Descriptor/DescriptorPool.h>
#include <Core/Descriptor/DescriptorLayout.h>
#include <Core/Descriptor/DescriptorSet.h>
#include "Core/Images/ImageView.h"
#include "Core/Texture.h"
#include "Scene/Compoments/Camera.h"
#include "Gui/InputEvent.h"
#include "Gui/Gui.h"

#include <stb_image.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <spdlog/spdlog.h>
#include <vk_mem_alloc.h>

#include <vector>
#include <optional>
#include <chrono>

// #ifdef _WIN32
// #include <minwindef.h>
// #include <WinUser.h>
// #include <windef.h>
// #endif


class Application {
    void initWindow(const char *name,  uint32_t width, uint32_t height);

    virtual void initGUI();



public:

    // static  Application * g_App{nullptr};
    
    Application(const char *name,  uint32_t width, uint32_t height);

    Application() : Application("Vulkan", 1920, 1080) {
    }

    virtual void prepare();

    virtual void inputEvent(const InputEvent &inputEvent);

    void mainloop();

    virtual  ~Application();

protected:
    void createRenderContext();


    virtual void update();

    virtual void getRequiredInstanceExtensions();

    inline void addDeviceExtension(const char *extension, bool optional = true) {
        deviceExtensions[extension] = optional;
    }

    inline void addInstanceExtension(const char *extension, bool optional = true) {
        instanceExtensions[extension] = optional;
    }


    virtual void initVk();

    virtual void drawFrame(RenderGraph &renderGraph, CommandBuffer &commandBuffer) = 0;

    virtual void updateScene();

    void updateGUI();

    virtual void onUpdateGUI();

    virtual void onMouseMove();

    virtual void onViewUpdated();


protected:
    VmaAllocator _allocator{};

    std::unique_ptr<Instance> _instance;


    std::unordered_map<const char *, bool> deviceExtensions;
    std::unordered_map<const char *, bool> instanceExtensions;
    std::vector<const char *> validationLayers{"VK_LAYER_KHRONOS_validation"};


    std::unique_ptr<Window> window{nullptr};

    std::unique_ptr<RenderContext> renderContext{nullptr};

    std::unique_ptr<Device> device{nullptr};

    VkSurfaceKHR surface{};

    std::unique_ptr<Camera> camera;

    std::unique_ptr<Scene> scene;

    std::unique_ptr<Gui> gui;

    float frameTimer{1};

    uint32_t width, height;

    //Camera related  variable begin
    struct {
        bool left{false};
        bool middle{false};
        bool right{false};
    } mouseButtons;

    glm::vec2 mousePos;

    glm::vec2 touchPos;

    glm::vec3 rotation;

    bool viewUpdated{false};

    float rotationSpeed{1};
    //Camera related  variable end




    VkFence fence{VK_NULL_HANDLE};
#ifdef NOEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif


    void handleMouseMove(float x, float y);
};
