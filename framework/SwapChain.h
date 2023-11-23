#pragma once

#include "Vulkan.h"

class Device;

class Surface;

class Window;

class FrameBuffer;

struct SwapchainProperties
{
    VkSwapchainKHR old_swapchain;
    uint32_t image_count{3};
    VkExtent2D extent{};
    VkSurfaceFormatKHR surface_format{};
    uint32_t array_layers;
    VkImageUsageFlags image_usage;
    VkSurfaceTransformFlagBitsKHR pre_transform;
    VkCompositeAlphaFlagBitsKHR composite_alpha;
    VkPresentModeKHR present_mode;
};

class SwapChain
{
public:
    inline VkSwapchainKHR getHandle()
    {
        return _swapChain;
    }

    inline VkSurfaceFormatKHR getFormat()
    {
        return _format;
    }

    inline VkFormat getImageFormat()
    {
        return _imageFormat;
    }

    inline VkExtent2D getExtent()
    {
        return _extent;
    }

    inline VkImageUsageFlags getUseage()
    {
        return prop.image_usage;
    }

    inline VkSurfaceKHR getSurface()
    {
        return _surface;
    }


    SwapChain(Device& device, VkSurfaceKHR surface, const VkExtent2D& extent);
    SwapChain(SwapChain& oldSwapChain, const VkExtent2D& newExtent);
    ~SwapChain();

    struct SwapChainSupportDetails
    {
        std::vector<VkSurfaceFormatKHR> formats{};
        std::vector<VkPresentModeKHR> presentModes{};
        VkSurfaceCapabilitiesKHR capabilities{};
    };

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);


    VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    SwapChainSupportDetails querySwapChainSupport();

    VkResult acquireNextImage(uint32& idx, VkSemaphore semaphore, VkFence fence);


    const std::vector<VkImage>& getImages();

    uint32_t getImageCount() const;

protected:
    VkSwapchainKHR _swapChain;

    Device& _device;

    VkSurfaceKHR _surface;


    VkSurfaceFormatKHR _format;

    VkFormat _imageFormat;

    VkExtent2D _extent;


    SwapchainProperties prop;

    std::vector<VkImage> images;

    uint32_t imageCount{2};
};
