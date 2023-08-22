//
// Created by 打工人 on 2023/3/19.
//
#pragma  once

#include "Vulkan.h"
#include "Images/ImageView.h"
#include "RenderFrame.h"

class Device;

class SwapChain;

class Window;

class FrameBuffer;

struct RenderContext {
public:
    RenderContext(Device &device, VkSurfaceKHR surface, Window &window);

    static RenderContext *g_context;

    // static RenderContext &getGlobalRenderContext() {
    //     return Singleton<RenderContext>::getInstance();
    // }
    VkFormat getSwapChainFormat() const;

    VkExtent2D getSwapChainExtent() const;

    uint32_t getSwapChainImageCount() const;

    //  inline uint32_t getBackBufferCount() { return backBufferCount; }

    //    inline const std::vector<VkImage>& getBackBufferImages() const
    //    {
    //        return _backBufferImages;
    //    }
    //    inline  const std::vector<VkImageView>& getBackBufferViews() const
    //    {
    //        return _backBufferImageViews;
    //    }
    void prepare();

    CommandBuffer &begin();

    void beginFrame();

    void waitFrame();

    RenderFrame &getActiveRenderFrame();

    RenderFrame &getRenderFrame(int idx);

    uint32_t getActiveFrameIndex() const;

    void submit(CommandBuffer &buffer, VkFence fence = VK_NULL_HANDLE);

    void createFrameBuffers(RenderPass &renderpass);

    FrameBuffer &getFrameBuffer(uint32_t idx);

    FrameBuffer &getFrameBuffer();

    VkSemaphore submit(const Queue &queue, const std::vector<CommandBuffer *> &commandBuffers, VkSemaphore waitSem,
                       VkPipelineStageFlags waitPiplineStage);

    void setActiveFrameIdx(int idx);

private:
    bool frameActive = false;
    VkSemaphore acquiredSem;
    bool prepared{false};
    uint32_t activeFrameIndex{0};


    VkInstance instance;
    std::vector<const char *> instanceLayers;
    std::vector<const char *> instanceExtensions;
    std::vector<const char *> appDeviceExtensions;
    std::vector<const char *> appInstanceExtensions;
    VkPhysicalDeviceFeatures2 *physicalDeviceFeatures2 = nullptr;

    Device &device;
    std::unique_ptr<SwapChain> swapchain;

    std::vector<std::unique_ptr<RenderFrame>> frames;
    std::vector<std::unique_ptr<FrameBuffer>> frameBuffers;
    VkExtent2D surfaceExtent;

    // 交换链图像信号 acquire-next
    //   VkSemaphore imageAcquireSem;

    struct {
        VkSemaphore presentFinishedSem;
        VkSemaphore renderFinishedSem;
    } semaphores;
};


