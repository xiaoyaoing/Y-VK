#pragma once

#include <VertexData.h>
#include <Buffer.h>
#include <CommandBuffer.h>
#include <Pipeline.h>
#include <FrameBuffer.h>
#include <RenderPass.h>
#include <Device.h>
#include <Images/Image.h>
#include <Queue.h>
#include <Window.h>
#include <Command/CommandPool.h>
#include <Instance.h>
#include <SwapChain.h>
#include <RenderTarget.h>
#include <RenderContext.h>
#include <Descriptor/DescriptorPool.h>
#include <Descriptor/DescriptorLayout.h>
#include <Descriptor/DescriptorSet.h>
#include <Images/ImageView.h>
#include <Images/Sampler.h>
#include <Mesh.h>
#include <Scene.h>
#include <API_VK.h>
#include <Camera.h>
#include <InputEvent.h>
#include <Gui.h>

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

class Application
{
    void initWindow(const char* name, int width, int height);

    virtual void initGUI();

    bool frameBufferResized{};

public:
    Application(const char* name, int width, int height);

    Application(): Application("Vulkan", 512, 512)
    {
    }

    virtual void prepare();

    void inputEvent(const InputEvent& inputEvent);

    void mainloop();

protected:
    // #ifdef _WIN32
    //     HWND setupWindow(HINSTANCE hinstance, WNDPROC wndproc);
    //     void handleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    // #endif()
    //* create functions
    //    virtual std::unique_ptr<RenderTarget> createRenderTarget(Image &image) {}

    virtual void draw(CommandBuffer& commandBuffer);

    virtual void createCommandBuffer();

    virtual void createCommandPool();

    virtual void createFrameBuffers();

    virtual void createRenderPass();

    virtual void createDepthStencil();

    void createRenderContext();

    virtual VkPhysicalDevice createPhysicalDevice();

    void createAllocator();

    virtual void update();

    virtual void bindUniformBuffers(CommandBuffer& commandBuffer)
    {
    }

    //*
    virtual void getRequiredInstanceExtensions();

    inline void addDeviceExtension(const char* extension, bool optional = true)
    {
        deviceExtensions[extension] = optional;
    }

    inline void addInstanceExtension(const char* extension, bool optional = true)
    {
        instanceExtensions[extension] = optional;
    }


    virtual void initVk();

    virtual void drawFrame();

    void drawRenderPasses(CommandBuffer& buffer, RenderTarget& renderTarget);

    virtual void updateScene();

    void updateGUI();

    virtual void onUpdateGUI();

    virtual void onMouseMove();

    virtual void onViewUpdated();

    virtual void buildCommandBuffers()
    {
    }

protected:
    VmaAllocator _allocator{};
    std::unique_ptr<Instance> _instance;
    ptr<Queue> _graphicsQueue, _presentQueue;

    std::vector<VkFramebuffer> _frameBuffers;

    // 指定怎么处理renderPass里面的attachment
    std::vector<LoadStoreInfo> loadStoreInfos{};

    std::unordered_map<const char*, bool> deviceExtensions;
    std::unordered_map<const char*, bool> instanceExtensions;
    std::vector<const char*> validationLayers{"VK_LAYER_KHRONOS_validation"};

    // std::unique_ptr<CommandPool> commandPool;
    std::vector<std::unique_ptr<CommandBuffer>> commandBuffers;
    //    ptr<Image> _depthImage;
    //    ptr<ImageView> _depthImageView;
    //    ptr<RenderPass> _renderPass;
    //
    std::unique_ptr<Window> window{nullptr};


    std::unique_ptr<RenderPipeline> renderPipeline{nullptr};

    std::unique_ptr<RenderContext> renderContext{nullptr};

    std::unique_ptr<Device> device{nullptr};

    //    std::unique_ptr<RenderPass> renderPass;

    VkSurfaceKHR surface{};

    std::vector<uint32_t> colorIdx{};
    std::vector<uint32_t> depthIdx{};

    VkPipelineLayout pipelineLayOut{};
    //    VkDescriptorSetLayout descriptorSetLayout{};
    std::unique_ptr<DescriptorLayout> descriptorLayout;

    std::unique_ptr<Scene> scene;

    std::unique_ptr<Camera> camera;

    std::unique_ptr<Gui> gui;

    float frameTimer{1};


    int width, height;

    //Camera related  variable begin
    struct
    {
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


    VkPipelineCache pipelineCache{VK_NULL_HANDLE};


    VkFence fence{VK_NULL_HANDLE};
#ifdef NOEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    void createRenderPipeline();

    void handleMouseMove(float x, float y);
};
