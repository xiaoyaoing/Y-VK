//
// Created by 打工人 on 2023/3/19.
//
#include "Vulkan.h"
#include "Engine/Images/ImageView.h"
#include "RenderFrame.h"

class Device;
class SwapChain;
class Window;

struct RenderContext{
public:
    RenderContext(ptr<Device> device, VkSurfaceKHR surface,ptr<Window> window);
    VkInstance							instance;
    std::vector<const char*>			instanceLayers;
    std::vector<const char*>			instanceExtensions;
    std::vector<const char*>			appDeviceExtensions;
    std::vector<const char*>			appInstanceExtensions;
    VkPhysicalDeviceFeatures2*			physicalDeviceFeatures2 = nullptr;

    ptr<Device>		device;
    ptr<SwapChain>	swapChain;
    std::vector<VkImage> _backBufferImages;
    std::vector<VkImageView> _backBufferImageViews;

    uint32_t backBufferCount;

public:
    inline VkFormat getSwapChainFormat() { return swapChain->getImageFormat();}
    inline VkExtent2D getSwapChainExtent() {return swapChain->getExtent();}
    inline uint32_t getBackBufferCount() {return backBufferCount;}
    inline const std::vector<VkImage>& getBackBufferImages() const
    {
        return _backBufferImages;
    }
    inline  const std::vector<VkImageView>& getBackBufferViews() const
    {
        return _backBufferImageViews;
    }


    CommandBuffer begin();
    void beginFrame();
    void waitFrame();
    RenderFrame & getActiveRenderFrame();
private:
    std::vector<RenderFrame> _frames;
    uint32_t activeFrameIndex;
    bool frameActive = false;
    VkSemaphore acquiredSem;
    bool prepared{false};

};



RenderContext * g_context;