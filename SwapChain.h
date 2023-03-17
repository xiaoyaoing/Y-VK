#pragma once
#include <Vulkan.h>
class Device;
class Surface;
class Window;
class SwapChain {
public:
    inline VkSwapchainKHR getHandle() {
        return _swapChain;
    }
    inline  VkSurfaceFormatKHR getFormat(){
        return _format;
    }
    inline VkFormat getImageFormat(){
        return _imageFormat;
    }
    inline  VkExtent2D getExtent(){
        return _extent;
    }
    SwapChain(ptr<Device> device,VkSurfaceKHR surface,ptr<Window>);
    bool initialize(ptr<Device> device,VkSurfaceKHR surface,ptr<Window>);
    struct SwapChainSupportDetails{
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
        VkSurfaceCapabilitiesKHR capabilities;
    };
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) ;

    VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    SwapChainSupportDetails querySwapChainSupport() ;
protected:
    VkSwapchainKHR _swapChain;
    ptr<Device> _device;
    VkSurfaceKHR  _surface;
    ptr<Window> _window;
    VkSurfaceFormatKHR _format;
    VkFormat _imageFormat;
    VkExtent2D _extent;
};
