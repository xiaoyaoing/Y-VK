#pragma once

#include "Vulkan.h"

class Device;

class Surface;

class Window;

class FrameBuffer;

class SwapChain {
public:
    inline VkSwapchainKHR getHandle() {
        return _swapChain;
    }

    inline VkSurfaceFormatKHR getFormat() {
        return _format;
    }

    inline VkFormat getImageFormat() {
        return _imageFormat;
    }

    inline VkExtent2D getExtent() {
        return _extent;
    }

    SwapChain(Device &device, VkSurfaceKHR surface, Window &window);

    bool initialize(Device &device, VkSurfaceKHR surface, Window &window);

    struct SwapChainSupportDetails {
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
        VkSurfaceCapabilitiesKHR capabilities;
    };

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);

    SwapChainSupportDetails querySwapChainSupport();

    VkResult acquireNextImage(uint32 &idx, VkSemaphore semaphore, VkFence fence);

    inline FrameBuffer &getFrameBuffer(int idx);

protected:
    VkSwapchainKHR _swapChain;
    Device &_device;
    VkSurfaceKHR _surface;
    Window &_window;
    VkSurfaceFormatKHR _format;
    VkFormat _imageFormat;
    VkExtent2D _extent;


};
