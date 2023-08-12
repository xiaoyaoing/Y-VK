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

    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    VK_CHECK_RESULT(
            vkCreateSemaphore(device.getHandle(), &semaphoreCreateInfo, nullptr, &semaphores.renderFinishedSem));
    VK_CHECK_RESULT(
            vkCreateSemaphore(device.getHandle(), &semaphoreCreateInfo, nullptr, &semaphores.presentFinishedSem));

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
        VK_CHECK_RESULT(swapchain->acquireNextImage(active_frame_index, semaphores.presentFinishedSem, VK_NULL_HANDLE));
    }
    frameActive = true;
    getActiveRenderFrame().reset();
}

void RenderContext::waitFrame() {
}

RenderFrame &RenderContext::getActiveRenderFrame() {
    assert(frameActive);
    assert(activeFrameIndex < frames.size());
    return *frames[activeFrameIndex];
}

void RenderContext::prepare() {
    if (swapchain) {
        surfaceExtent = swapchain->getExtent();

        VkExtent3D extent{surfaceExtent.width, surfaceExtent.height, 1};

        for (auto &image_handle: swapchain->getImages()) {
            auto swapchain_image = Image{
                    device, image_handle,
                    extent,
                    swapchain->getImageFormat(),
                    swapchain->getUseage()};
            auto render_target = RenderTarget::defaultRenderTargetCreateFunction(std::move(swapchain_image));
            frames.emplace_back(std::make_unique<RenderFrame>(device, std::move(render_target)));
        }
    } else {
        swapchain = nullptr;
    }
}

uint32_t RenderContext::getActiveFrameIndex() const {
    return 0;
}

void RenderContext::submit(CommandBuffer &buffer) {
}

VkFormat RenderContext::getSwapChainFormat() const {
    return swapchain->getImageFormat();
}

VkExtent2D RenderContext::getSwapChainExtent() const {
    return swapchain->getExtent();
}

FrameBuffer &RenderContext::getFrameBuffer(uint32_t idx) {
    return *frameBuffers[idx];
}

FrameBuffer &RenderContext::getFrameBuffer() {
    return getFrameBuffer(this->active_frame_index);
}

VkSemaphore
RenderContext::submit(const Queue &queue, const std::vector<CommandBuffer *> &commandBuffers, VkSemaphore /*waitSem*/,
                      VkPipelineStageFlags waitPiplineStage) {
    std::vector<VkCommandBuffer> cmdBufferHandles(commandBuffers.size(), VK_NULL_HANDLE);

    std::transform(commandBuffers.begin(), commandBuffers.end(), cmdBufferHandles.begin(),
                   [](const CommandBuffer *buffer) -> VkCommandBuffer {
                       return buffer->getHandle();
                   });


    RenderFrame &frame = getActiveRenderFrame();
    VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &semaphores.presentFinishedSem;
    submitInfo.pWaitDstStageMask = &waitPiplineStage;

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &semaphores.renderFinishedSem;

    VkFence fence;
    queue.submit({submitInfo}, fence);

    if (swapchain) {
        VkSwapchainKHR vk_swapchain = swapchain->getHandle();

        VkPresentInfoKHR present_info{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};

        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &semaphores.renderFinishedSem;
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
//    if (imageAcquireSem) {
//        //        release_owned_semaphore(acquired_semaphore);
//        imageAcquireSem = VK_NULL_HANDLE;
//    }
    frameActive = false;

    return VK_NULL_HANDLE;
}

void RenderContext::createFrameBuffers(RenderPass &renderPass) {
    if (!frameBuffers.empty()) {
        return;
    }
    for (auto &renderFrame: frames) {
        frameBuffers.emplace_back(std::make_unique<FrameBuffer>(device, renderFrame->getRenderTarget(), renderPass));
    }
}

uint32_t RenderContext::getSwapChainImageCount() const {
    return swapchain->getImageCount();
}

RenderFrame &RenderContext::getRenderFrame(int idx) {
    return *frames[idx];
}
