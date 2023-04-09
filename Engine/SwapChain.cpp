#include "SwapChain.h"
#include "Engine/Device.h"
#include "Engine/Queue.h"
#include <utility>
#include "Window.h"
SwapChain::SwapChain(ptr<Device> device, VkSurfaceKHR surface, ptr<Window> window) {
    ASSERT(initialize(device, surface,window), "Failed to create swapChain");

}

bool SwapChain::initialize(ptr<Device> device, VkSurfaceKHR surface, ptr<Window> window) {
    _device = std::move(device),
   _surface = surface; _window = std::move(window);
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport();
    auto surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    auto presentMode = choosePresentMode(swapChainSupport.presentModes);
    auto extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = 2;

    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }


    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    //QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    //  uint32_t queueFamilyIndices[]{indices.graphicsFamily.value(), indices.presentFamily.value()};
    uint32_t graphicsFamily = _device->getQueueByFlag(VK_QUEUE_GRAPHICS_BIT, 0)->getFamilyIndex();
    uint32_t presentFamily = _device->getPresentQueue(0)->getFamilyIndex();

    if (graphicsFamily != presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = new uint32_t[]{graphicsFamily, presentFamily};
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;


    if(vkCreateSwapchainKHR(_device->getHandle(), &createInfo, nullptr, &_swapChain) !=VK_SUCCESS)
        return false;
    _format = surfaceFormat;
    _imageFormat = surfaceFormat.format;
    _extent = extent;
    return true;
}

SwapChain::SwapChainSupportDetails SwapChain::querySwapChainSupport() {
    auto physicalDevice = _device->getPhysicalDevice();
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, _surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, _surface, &formatCount, nullptr);
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, _surface, &formatCount, details.formats.data());
    }

    uint32_t presentCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, _surface, &presentCount, nullptr);
    if (presentCount != 0) {
        details.presentModes.resize(presentCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, _surface, &presentCount, details.presentModes.data());
    }
    return details;
}


VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
    for (const auto &availableFormat: availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace ==
            VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(_window->getHandle(), &width, &height);
        VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)};
        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                                        capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height,
                                         capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        return actualExtent;
    }
}

VkPresentModeKHR SwapChain::choosePresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
    for (const auto &availablePresentMode: availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkResult SwapChain::acquireNextImage(uint32 &idx, VkSemaphore semaphore, VkFence fence) {
    return vkAcquireNextImageKHR(device.get_handle(), handle, std::numeric_limits<uint64_t>::max(), image_acquired_semaphore, fence, &image_index);

}

