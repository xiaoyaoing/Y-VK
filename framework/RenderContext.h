//
// Created by 打工人 on 2023/3/19.
//
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

    static RenderContext &getGlobalRenderContext() {
        return Singleton<RenderContext>::getInstance();
    }

public:
    inline VkFormat getSwapChainFormat() { return swapchain->getImageFormat(); }

    inline VkExtent2D getSwapChainExtent() { return swapchain->getExtent(); }

    inline uint32_t getBackBufferCount() { return backBufferCount; }

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

    uint32_t getActiveFrameIndex() const;

    void submit(CommandBuffer &buffer);

    FrameBuffer &getFrameBuffer(uint32_t idx);

    FrameBuffer &getFrameBuffer();

    VkSemaphore submit(const Queue &queue, const std::vector<CommandBuffer *> &commandBuffers, VkSemaphore waitSem,
                       VkPipelineStageFlags waitPiplineStage);


private:
    uint32_t activeFrameIndex;
    bool frameActive = false;
    VkSemaphore acquiredSem;
    bool prepared{false};

    VkInstance instance;
    std::vector<const char *> instanceLayers;
    std::vector<const char *> instanceExtensions;
    std::vector<const char *> appDeviceExtensions;
    std::vector<const char *> appInstanceExtensions;
    VkPhysicalDeviceFeatures2 *physicalDeviceFeatures2 = nullptr;

    Device &device;
    std::unique_ptr<SwapChain> swapchain;

    std::vector<std::unique_ptr<RenderFrame>> frames;
    uint32_t active_frame_index{0};
    std::vector<std::unique_ptr<FrameBuffer>> frameBuffers;
    VkExtent2D surfaceExtent;

    //交换链图像信号 acquire-next
    VkSemaphore imageAcquireSem;
};

RenderContext *g_context;