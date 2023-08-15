#include "SwapChain.h"
#include "Device.h"
#include "Window.h"
SwapChain::SwapChain(Device &  device, VkSurfaceKHR surface, Window &  window):_device(device),_window(window),_surface(surface) {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport();
    auto surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    auto presentMode = choosePresentMode(swapChainSupport.presentModes);
    auto extent = chooseSwapExtent(swapChainSupport.capabilities);



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
    createInfo.compositeAlpha =VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    //QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    //  uint32_t queueFamilyIndices[]{indices.graphicsFamily.value(), indices.presentFamily.value()};
    uint32_t graphicsFamily = _device.getQueueByFlag(VK_QUEUE_GRAPHICS_BIT, 0).getFamilyIndex();
    uint32_t presentFamily = _device.getPresentQueue(0).getFamilyIndex();

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

    VK_CHECK_RESULT(vkCreateSwapchainKHR(_device.getHandle(), &createInfo, nullptr, &_swapChain));
    _format = surfaceFormat;
    _imageFormat = surfaceFormat.format;
    _extent = extent;

    //create swapchain images
    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(_device.getHandle(), _swapChain, &imageCount, nullptr));
    images.resize(imageCount);
    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(_device.getHandle(), _swapChain, &imageCount, images.data()));

}

bool SwapChain::initialize(Device &  device, VkSurfaceKHR surface, Window &  window) {
    return false;
}


SwapChain::SwapChainSupportDetails SwapChain::querySwapChainSupport() {
    auto physicalDevice = _device.getPhysicalDevice();
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
        glfwGetFramebufferSize(_window.getHandle(), &width, &height);
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
    return vkAcquireNextImageKHR(_device.getHandle(),_swapChain, std::numeric_limits<uint64_t>::max(), semaphore, fence, &idx);

}

const std::vector<VkImage> &SwapChain::getImages()
{
    return images;
}

uint32_t SwapChain::getImageCount() const {
    return imageCount;
}
