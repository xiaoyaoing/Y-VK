#pragma once

#include <GLFW/glfw3.h>
#include <vector>
#include <optional>

#include "Engine/Vulkan.h"
#include "Engine/VertexData.h"
#include "Engine/Buffer.h"
#include "Engine/CommandBuffer.h"
#include "Engine/Pipeline.h"
#include "Engine/FrameBuffer.h"
#include "Engine/RenderPass.h"
#include "Engine/Device.h"
#include <Engine/Images/Image.h>
#include "Engine/Queue.h"
#include "Engine/Window.h"
#include <Engine/Command/CommandPool.h>
#include "Engine/Instance.h"
#include "Engine/SwapChain.h"
#include <Engine/RenderContext.h>

#include "ext/stb_image/stb_image.h"
#include "Engine/Images/ImageView.h"
#include "Engine/Images/Sampler.h"
#include "Engine/Mesh.h"
#include "RenderTarget.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include <Engine/Descriptor/DescriptorPool.h>
#include <Engine/Descriptor/DescriptorLayout.h>
#include <Engine/Descriptor/DescriptorSet.h>

#include <chrono>
class Application {
    Application(const char * name,int width,int height);
    void run() {
        initWindow();
        initVk();
        mainloop();
       // cleanup();
    };
    virtual void initWindow(const char * name,int width,int height) {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        _window = std::make_shared<Window>(glfwCreateWindow(width,height ,name, nullptr, nullptr));
        glfwSetFramebufferSizeCallback(_window->getHandle(), [](GLFWwindow *window, int width, int height) {
            auto *app = reinterpret_cast<Application *>(glfwGetWindowUserPointer(window));
            app->frameBufferResized = true;
        });
    }

    virtual void mainloop(){
        while (!glfwWindowShouldClose(_window->getHandle())) {
            glfwPollEvents();
            drawFrame();
        }
    }
    bool frameBufferResized;
protected:
    //* create functions
    virtual  std::unique_ptr<RenderTarget> createRenderTarget(Image & image) {}
    virtual  void draw(CommandBuffer & commandBuffer,RenderTarget & target);
    virtual  void createCommandBuffer();
    virtual  void createCommandPool();
    virtual  void createFrameBuffers();
    virtual  void createRenderPass();
    virtual  void createDepthStencil();
    virtual  VkPhysicalDevice  createPhysicalDevice();
    void createAllocator();
    virtual void update();
    void present(uint32_t bufferIndex);
    //*
    virtual void getRequiredExtensions();
    inline void addDeviceExtension(const char *extension, bool optional = true){
        deviceExtensions[extension] = optional;
    }
    inline void addInstanceExtension(const char *extension, bool optional = true){
        instanceExtensions[extension] = optional;
    }

    virtual  void initVk();

protected:
    VmaAllocator _allocator;
    ptr<Device> _device;
    ptr<Instance> _instance;
    ptr<Queue> _graphicsQueue,_presentQueue;


    std::vector<VkFramebuffer> _frameBuffers;
    ptr<Window> _window;
    ptr<RenderContext>  _context;

    std::unordered_map<const char *, bool> deviceExtensions;
    std::unordered_map<const char *, bool> instanceExtensions;
    std::vector<const char *> validationLayers;

    ptr<CommandPool> commandPool;
    std::vector<ptr<CommandBuffer>> commandBuffers;
    ptr<Image> _depthImage;
    ptr<ImageView> _depthImageView;
    ptr<RenderPass> _renderPass;

#ifdef  NOEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
};
