#include "SwapChain.h"
#include "Core/Device/Device.h"
#include "PlatForm/Window.h"


SwapChain::SwapChain(Device& device, VkSurfaceKHR surface, const VkExtent2D& extent): _device(device), _surface(surface)
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport();
    auto surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    auto presentMode = choosePresentMode(swapChainSupport.presentModes);


    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }
    
    // Check if the extent is supported by the surface
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_device.getPhysicalDevice(), surface, &surfaceCapabilities);

    VkExtent2D imageExtent;
    imageExtent.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, extent.width));
    imageExtent.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, extent.height));

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.imageExtent = imageExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |VK_IMAGE_USAGE_TRANSFER_DST_BIT ;

    prop.image_usage = createInfo.imageUsage;
    prop.extent = imageExtent;
    prop.surface_format = surfaceFormat;

    //QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    //  uint32_t queueFamilyIndices[]{indices.graphicsFamily.value(), indices.presentFamily.value()};
    uint32_t graphicsFamily = _device.getQueueByFlag(VK_QUEUE_GRAPHICS_BIT, 0).getFamilyIndex();
    uint32_t presentFamily = _device.getPresentQueue(0).getFamilyIndex();

    if (graphicsFamily != presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = new uint32_t[]{graphicsFamily, presentFamily};
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.presentMode = presentMode;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VkSwapchainKHR swapchainKhr;

    VK_CHECK_RESULT(vkCreateSwapchainKHR(_device.getHandle(), &createInfo, nullptr, &_swapChain));
    // VK_CHECK_RESULT(vkCreateSwapchainKHR(_device.getHandle(), &createInfo, nullptr, &swapchainKhr));
    _format = surfaceFormat;
    _imageFormat = surfaceFormat.format;
    _extent = imageExtent;

    //create swapchain images
    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(_device.getHandle(), _swapChain, &imageCount, nullptr));
    images.resize(imageCount);
    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(_device.getHandle(), _swapChain, &imageCount, images.data()));
}

SwapChain::SwapChain(SwapChain& oldSwapChain, const VkExtent2D& newExtent): SwapChain(
    oldSwapChain._device, oldSwapChain._surface, newExtent)
{
}

SwapChain::~SwapChain()
{
    if (_swapChain != VK_NULL_HANDLE)
        vkDestroySwapchainKHR(_device.getHandle(), _swapChain, nullptr);
}


SwapChain::SwapChainSupportDetails SwapChain::querySwapChainSupport()
{
    auto physicalDevice = _device.getPhysicalDevice();
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, _surface, &details.capabilities);

    uint32_t formatCount;







    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, _surface, &formatCount, nullptr);
    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, _surface, &formatCount, details.formats.data());
    }

    uint32_t presentCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, _surface, &presentCount, nullptr);
    if (presentCount != 0)
    {
        details.presentModes.resize(presentCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, _surface, &presentCount, details.presentModes.data());
    }
    return details;
}


VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace ==
            VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }
    return availableFormats[0];
}


VkPresentModeKHR SwapChain::choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkResult SwapChain::acquireNextImage(uint32& idx, VkSemaphore semaphore, VkFence fence)
{
    return vkAcquireNextImageKHR(_device.getHandle(), _swapChain, std::numeric_limits<uint64_t>::max(), semaphore,
                                 fence, &idx);
}

const std::vector<VkImage>& SwapChain::getImages()
{
    return images;
}

uint32_t SwapChain::getImageCount() const
{
    return imageCount;
}
