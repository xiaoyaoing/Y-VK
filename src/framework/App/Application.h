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

#include <imgui.h>

#include "Common/Timer.h"
#include "Core/View.h"
#include "RenderPasses/RenderPassBase.h"
#include "Scene/SceneLoader/SceneLoadingConfig.h"

// #ifdef _WIN32
// #include <minwindef.h>
// #include <WinUser.h>
// #include <windef.h>
// #endif

#define EXAMPLE_MAIN                  \
    int main(int argc, char** argv) { \
        Example app;                  \
        app.prepare();                \
        app.mainloop();               \
        return 0;                     \
    }

class Application {
    void initWindow(const char* name, uint32_t width, uint32_t height);

    virtual void initGUI();

public:
    Application(const char* name, uint32_t width, uint32_t height);
    Application() : Application("Vulkan", 1920, 1080) {
    }

    virtual void prepare();
    virtual void inputEvent(const InputEvent& inputEvent);
    virtual ~Application();

    void         setFocused(bool focused);
    void         mainloop();
    void         onResize(uint32_t width, uint32_t height);
    void         initView();
    void         loadScene(const std::string& path);
    virtual void onSceneLoaded();

protected:
    inline void addDeviceExtension(const char* extension, bool optional = true) {
        deviceExtensions[extension] = optional;
    }
    inline void addInstanceExtension(const char* extension, bool optional = true) {
        instanceExtensions[extension] = optional;
    }

    virtual void update();
    virtual void getRequiredInstanceExtensions();
    virtual void initVk();
    virtual void drawFrame(RenderGraph& rg) = 0;
    virtual void updateScene();
    virtual void onUpdateGUI();
    virtual void onMouseMove();
    virtual void onViewUpdated();
    virtual void preparePerViewData();

    void updateGUI();
    void createRenderContext();

    void handleSaveImage(RenderGraph& graph);
    void resetImageSave();
    //void loadScene(const std::string & path);
protected:
    VmaAllocator _allocator{};

    float deltaTime{0};

    Timer timer;

    std::unique_ptr<Instance>             _instance;
    std::unordered_map<const char*, bool> deviceExtensions;
    std::unordered_map<const char*, bool> instanceExtensions;
    std::vector<const char*>              validationLayers{"VK_LAYER_KHRONOS_validation"};
    std::unique_ptr<Window>               window{nullptr};
    std::unique_ptr<RenderContext>        renderContext{nullptr};
    std::unique_ptr<Device>               device{nullptr};
    std::shared_ptr<Camera>               camera;

    std::unique_ptr<Scene> scene;
    std::unique_ptr<Scene> sceneAsync{nullptr};
    std::unique_ptr<View>  view;
    std::unique_ptr<Gui>   gui;

    VkSurfaceKHR surface{};

    uint32_t  mWidth, mHeight;
    bool      m_focused{true};
    glm::vec2 mousePos;

    glm::vec2 touchPos;

    glm::vec3 rotation;

    bool viewUpdated{false};

    float rotationSpeed{1};

    bool               enableGui{true};
    uint32_t           frameCounter{0};
    SceneLoadingConfig sceneLoadingConfig;

    struct ImageSave {
        bool                    savePng = false;
        bool                    saveExr = false;
        std::unique_ptr<Buffer> buffer{nullptr};
    } imageSave;

    bool saveCamera{false};

    void handleMouseMove(float x, float y);

private:
    std::string               mPresentTexture = RENDER_VIEW_PORT_IMAGE_NAME;
    std::vector<std::string>  mCurrentTextures{RENDER_VIEW_PORT_IMAGE_NAME};
    const char*               mAppName;
    std::unique_ptr<PassBase> mPostProcessPass{};
    //Camera related  variable end

    VkFence fence{VK_NULL_HANDLE};
#ifdef NOEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
};