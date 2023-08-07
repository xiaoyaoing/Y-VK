//
// Created by 打工人 on 2023/3/30.
//
#include <RenderContext.h>
#include <CommandBuffer.h>
#include <Device.h>
#include <SwapChain.h>
#include <Queue.h>
#include <Images/Image.h>
#include <RenderTarget.h>
#include <FrameBuffer.h>

RenderContext::RenderContext(Device &device, VkSurfaceKHR surface, Window &window) : device(device) {
    swapchain = std::make_unique<SwapChain>(device, surface, window);
    if (swapchain) {
        surfaceExtent = swapchain->getExtent();

        VkExtent3D extent{surfaceExtent.width, surfaceExtent.height, 1};

        for (auto &image_handle: swapchain->getImages()) {
            auto swapChainImage = Image(device, image_handle,
                                        extent,
                                        swapchain->getImageFormat(),
                                        swapchain->getUseage());
            auto render_target = RenderTarget::defaultRenderTargetCreateFunction(std::move(swapChainImage));
            frames.emplace_back(std::make_unique<RenderFrame>(device, std::move(render_target)));
        }
    }
}

CommandBuffer &RenderContext::begin() {
    assert(prepared && "RenderContext not prepared for rendering, call prepare()");

    if (!frameActive) {
        beginFrame();
    }

    if (acquiredSem == VK_NULL_HANDLE) {
        throw std::runtime_error("Couldn't begin frame");
    }
    const auto &queue = device.getQueueByFlag(VK_QUEUE_GRAPHICS_BIT, 0);
    return getActiveRenderFrame().requestCommandBuffer(queue);
}

void RenderContext::beginFrame() {
    assert(active_frame_index < frames.size());
    auto &prev_frame = *frames[active_frame_index];
    if (swapchain) {
        VK_CHECK_RESULT(swapchain->acquireNextImage(active_frame_index, imageAcquireSem, VK_NULL_HANDLE));
    }
    frameActive = true;
    getActiveRenderFrame().reset();

}

void RenderContext::waitFrame() {
}

RenderFrame &RenderContext::getActiveRenderFrame() {
    assert(activeFrameIndex < _frames.size());
    return *frames[activeFrameIndex];
}

void RenderContext::prepare() {
    if (swapchain) {
        surface_extent = swapchain->get_extent();

        VkExtent3D extent{surface_extent.width, surface_extent.height, 1};

        for (auto &image_handle: swapchain->get_images()) {
            auto swapchain_image = Image{
                    device, image_handle,
                    extent,
                    swapchain->get_format(),
                    swapchain->get_usage()};
            auto render_target = create_render_target_func(std::move(swapchain_image));
            frames.emplace_back(std::make_unique<RenderFrame>(device, std::move(render_target), thread_count));
        }
    } else {
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

uint32_t RenderContext::getActiveFrameIndex() const {
    return 0;
}

void RenderContext::submit(CommandBuffer &buffer) {
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkSemaphore waitSemaphores[] = {acquiredSem};
    VkPipelineStageFlags waitStages[] =
            {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    VkCommandBuffer submitCommandBuffers[] = {commandBuffers[curFrameCount]->getHandle()};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = submitCommandBuffers;

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[curFrameCount]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapChains[] = {_swapChain->getHandle()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    //vkDeviceWaitIdle(device);
    //std::this_thread::sleep_for(std::chrono::milliseconds (10));

    result = vkQueuePresentKHR(presentQueue->getHandle(), &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || frameBufferResized) {
        frameBufferResized = false;
        reCreateSwapChain();
        return;
    }

    curFrameCount = (curFrameCount + 1) % MAX_FRAMES_IN_FLIGHT;
}

FrameBuffer &RenderContext::getFrameBuffer(uint32_t idx) {
    return <#initializer#>;
}

FrameBuffer &RenderContext::getFrameBuffer() {
    return <#initializer#>;
}

VkSemaphore
RenderContext::submit(const Queue &queue, const std::vector<CommandBuffer *> &commandBuffers, VkSemaphore waitSem,
                      VkPipelineStageFlags waitPiplineStage) {
    std::vector<VkCommandBuffer> cmdBufferHandles = getHandles<CommandBuffer, VkCommandBuffer>(commandBuffers);


    RenderFrame &frame = getActiveRenderFrame();
    VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    VkSemaphore signalSem = frame.getSem();
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &waitSem;
    submitInfo.pWaitDstStageMask = &waitPiplineStage;

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &signalSem;

    VkFence fence;
    queue.submit({submitInfo}, fence);

    if (swapchain) {
        VkSwapchainKHR vk_swapchain = swapchain->getHandle();

        VkPresentInfoKHR present_info{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};

        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &signalSem;
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &vk_swapchain;
        present_info.pImageIndices = &active_frame_index;

//        VkDisplayPresentInfoKHR disp_present_info{};
//        if (device.is_extension_supported(VK_KHR_DISPLAY_SWAPCHAIN_EXTENSION_NAME) &&
//            window.get_display_present_info(&disp_present_info, surface_extent.width, surface_extent.height)) {
//            // Add display present info if supported and wanted
//            present_info.pNext = &disp_present_info;
//        }

        VK_CHECK_RESULT(queue.present(present_info));

//        if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR)
//        {
//            handle_surface_changes();
//        }
    }

    // Frame is not active anymore
    if (imageAcquireSem) {
//        release_owned_semaphore(acquired_semaphore);
        imageAcquireSem = VK_NULL_HANDLE;
    }
    frameActive = false;

    return VK_NULL_HANDLE;
}
