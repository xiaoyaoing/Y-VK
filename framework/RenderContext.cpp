//
// Created by 打工人 on 2023/3/30.
//
#include <RenderContext.h>
#include <CommandBuffer.h>
#include <Device.h>
#include <SwapChain.h>
#include <Queue.h>

RenderContext::RenderContext(Device &device, VkSurfaceKHR surface, Window &window)
{
    swapchain = std::make_unique<SwapChain>(device, surface, window);
}

CommandBuffer & RenderContext::begin()
{
    assert(prepared && "RenderContext not prepared for rendering, call prepare()");

    if (!frameActive)
    {
        beginFrame();
    }

    if (acquiredSem == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Couldn't begin frame");
    }
    const auto &queue = device.getQueueByFlag(VK_QUEUE_GRAPHICS_BIT, 0);
    return getActiveRenderFrame().requestCommandBuffer(queue);
}

void RenderContext::beginFrame()
{
    // return getActiveRenderFrame().requestCommandBuffer(queue);
}

void RenderContext::waitFrame()
{
}

RenderFrame &RenderContext::getActiveRenderFrame()
{
    assert(activeFrameIndex < _frames.size());
    return _frames[activeFrameIndex];
}

void RenderContext::prepare()
{
    if (swapchain)
    {
        surface_extent = swapchain->get_extent();

        VkExtent3D extent{surface_extent.width, surface_extent.height, 1};

        for (auto &image_handle : swapchain->get_images())
        {
            auto swapchain_image = Image{
                device, image_handle,
                extent,
                swapchain->get_format(),
                swapchain->get_usage()};
            auto render_target = create_render_target_func(std::move(swapchain_image));
            frames.emplace_back(std::make_unique<RenderFrame>(device, std::move(render_target), thread_count));
        }
    }
    else
    {
        // Otherwise, create a single RenderFrame
        swapchain = nullptr;

        auto color_image = core::Image{device,
                                       VkExtent3D{surface_extent.width, surface_extent.height, 1},
                                       DEFAULT_VK_FORMAT, // We can use any format here that we like
                                       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                       VMA_MEMORY_USAGE_GPU_ONLY};

        auto render_target = create_render_target_func(std::move(color_image));
        frames.emplace_back(std::make_unique<RenderFrame>(device, std::move(render_target), thread_count));
    }

    this->create_render_target_func = create_render_target_func;
    this->thread_count = thread_count;
    this->prepared = true;
}
