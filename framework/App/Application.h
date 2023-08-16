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

#include <stb_image.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <spdlog/spdlog.h>

#include <vector>
#include <optional>
#include <chrono>
#include "Scene.h"
#include "vk_mem_alloc.h"

class Application {

    void initWindow(const char *name, int width, int height);

    virtual void initGUI();


    bool frameBufferResized{};

public:
    Application(const char *name, int width, int height);

    void prepare();

    virtual void mainloop() {
        while (!glfwWindowShouldClose(window->getHandle())) {
            glfwPollEvents();
            drawFrame();
        }
    }

protected:

    //* create functions
//    virtual std::unique_ptr<RenderTarget> createRenderTarget(Image &image) {}

    virtual void draw(CommandBuffer &commandBuffer, RenderTarget &renderTarget);

    virtual void createCommandBuffer();

    virtual void createCommandPool();

    virtual void createFrameBuffers();

    virtual void createRenderPass();

    virtual void createDepthStencil();

    virtual void createPipeline();

    void createRenderContext();

    virtual VkPhysicalDevice createPhysicalDevice();

    void createAllocator();

    virtual void update();

    void present(uint32_t bufferIndex);

    //*
    virtual void getRequiredExtensions();

    inline void addDeviceExtension(const char *extension, bool optional = true) {
        deviceExtensions[extension] = optional;
    }

    inline void addInstanceExtension(const char *extension, bool optional = true) {
        instanceExtensions[extension] = optional;
    }

    virtual void initVk();


    virtual void drawFrame();

    void drawRenderPasses(CommandBuffer &buffer, RenderTarget &renderTarget);


    virtual void updateScene();

    virtual void updateGUI();

protected:
    VmaAllocator _allocator{};
    ptr<Instance> _instance;
    ptr<Queue> _graphicsQueue, _presentQueue;

    std::vector<VkFramebuffer> _frameBuffers;

    std::unordered_map<const char *, bool> deviceExtensions;
    std::unordered_map<const char *, bool> instanceExtensions;
    std::vector<const char *> validationLayers{"VK_LAYER_KHRONOS_validation"};

    ptr<CommandPool> commandPool;
    std::vector<std::unique_ptr<CommandBuffer>> commandBuffers;
    ptr<Image> _depthImage;
    ptr<ImageView> _depthImageView;
    ptr<RenderPass> _renderPass;

    std::unique_ptr<Window> window{nullptr};

    std::unique_ptr<Pipeline> graphicsPipeline{nullptr};

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

    VkFence fence{VK_NULL_HANDLE};
#ifdef NOEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    void createRenderPipeline();
};
