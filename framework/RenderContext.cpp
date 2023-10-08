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
#include <Buffer.h>

#include "Common/ResourceCache.h"
#include "Images/Sampler.h"

RenderContext* RenderContext::g_context = nullptr;

RenderContext::RenderContext(Device& device, VkSurfaceKHR surface, Window& window)
    : device(device)
{
    swapchain = std::make_unique<SwapChain>(device, surface, window);
    if (swapchain)
    {
        surfaceExtent = swapchain->getExtent();

        VkExtent3D extent{surfaceExtent.width, surfaceExtent.height, 1};

        for (auto& image_handle : swapchain->getImages())
        {
            auto swapChainImage = Image(device, image_handle,
                                        extent,
                                        swapchain->getImageFormat(),
                                        swapchain->getUseage());
            hwTextures.emplace_back(device, image_handle, extent, swapchain->getImageFormat(), swapchain->getUseage());
        }
    }

    VkSemaphoreCreateInfo semaphoreCreateInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VK_CHECK_RESULT(
        vkCreateSemaphore(device.getHandle(), &semaphoreCreateInfo, nullptr, &semaphores.renderFinishedSem));
    VK_CHECK_RESULT(
        vkCreateSemaphore(device.getHandle(), &semaphoreCreateInfo, nullptr, &semaphores.presentFinishedSem));

    renderGraph = std::make_unique<RenderGraph>(device);
}

CommandBuffer& RenderContext::begin()
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
    auto& queue = device.getQueueByFlag(VK_QUEUE_GRAPHICS_BIT, 0);
    // return getActiveRenderFrame().requestCommandBuffer(queue);
}

void RenderContext::beginFrame()
{
    // assert(activeFrameIndex < frames.size());
    if (swapchain)
    {
        VK_CHECK_RESULT(swapchain->acquireNextImage(activeFrameIndex, semaphores.presentFinishedSem, VK_NULL_HANDLE));
    }
    frameActive = true;
    //   getActiveRenderFrame().reset();
}

void RenderContext::waitFrame()
{
}

// RenderFrame &RenderContext::getActiveRenderFrame() {
//     assert(frameActive);
//     assert(activeFrameIndex < frames.size());
//     return *frames[activeFrameIndex];
// }

void RenderContext::prepare()
{
    if (swapchain)
    {
        surfaceExtent = swapchain->getExtent();

        VkExtent3D extent{surfaceExtent.width, surfaceExtent.height, 1};

        for (auto& image_handle : swapchain->getImages())
        {
            // auto swapchain_image = Image{
            //         device, image_handle,
            //         extent,
            //         swapchain->getImageFormat(),
            //         swapchain->getUseage()};
            // auto render_target = RenderTarget::defaultRenderTargetCreateFunction(std::move(swapchain_image));
            // frames.emplace_back(std::make_unique<RenderFrame>(device, std::move(render_target)));
        }
    }
    else
    {
        swapchain = nullptr;
    }
}

uint32_t RenderContext::getActiveFrameIndex() const
{
    return activeFrameIndex;
}

void RenderContext::submit(CommandBuffer& buffer, VkFence fence)
{
    std::vector<VkCommandBuffer> cmdBufferHandles{buffer.getHandle()};


    auto queue = device.getQueueByFlag(VK_QUEUE_GRAPHICS_BIT, 0);

    VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &semaphores.presentFinishedSem;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &semaphores.renderFinishedSem;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = cmdBufferHandles.data();

    queue.submit({submitInfo}, fence);

    if (swapchain)
    {
        VkSwapchainKHR vk_swapchain = swapchain->getHandle();

        VkPresentInfoKHR present_info{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};

        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &semaphores.renderFinishedSem;
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &vk_swapchain;
        present_info.pImageIndices = &activeFrameIndex;


        VK_CHECK_RESULT(queue.present(present_info));
        // LOGE("Presented")

        frameActive = false;
    }

    queue.wait();
}

VkFormat RenderContext::getSwapChainFormat() const
{
    return swapchain->getImageFormat();
}

VkExtent2D RenderContext::getSwapChainExtent() const
{
    return swapchain->getExtent();
}

FrameBuffer& RenderContext::getFrameBuffer(uint32_t idx)
{
    return *frameBuffers[idx];
}

FrameBuffer& RenderContext::getFrameBuffer()
{
    return getFrameBuffer(this->activeFrameIndex);
}

VkSemaphore
RenderContext::submit(const Queue& queue, const std::vector<CommandBuffer*>& commandBuffers, VkSemaphore /*waitSem*/,
                      VkPipelineStageFlags waitPiplineStage)
{
    std::vector<VkCommandBuffer> cmdBufferHandles(commandBuffers.size(), VK_NULL_HANDLE);

    std::transform(commandBuffers.begin(), commandBuffers.end(), cmdBufferHandles.begin(),
                   [](const CommandBuffer* buffer) -> VkCommandBuffer
                   {
                       return buffer->getHandle();
                   });


    //  RenderFrame &frame = getActiveRenderFrame();
    VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &semaphores.presentFinishedSem;
    submitInfo.pWaitDstStageMask = &waitPiplineStage;

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &semaphores.renderFinishedSem;

    VkFence fence;
    queue.submit({submitInfo}, fence);

    if (swapchain)
    {
        VkSwapchainKHR vk_swapchain = swapchain->getHandle();

        VkPresentInfoKHR present_info{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};

        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &semaphores.renderFinishedSem;
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &vk_swapchain;
        present_info.pImageIndices = &activeFrameIndex;

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

void RenderContext::createFrameBuffers(RenderPass& renderPass)
{
    if (!frameBuffers.empty())
    {
        return;
    }
    // for (auto &renderFrame: frames) {
    //     frameBuffers.emplace_back(std::make_unique<FrameBuffer>(device, renderFrame->getRenderTarget(), renderPass));
    // }
}

uint32_t RenderContext::getSwapChainImageCount() const
{
    return swapchain->getImageCount();
}

// RenderFrame &RenderContext::getRenderFrame(int idx) {
//     return *frames[idx];
// }

void RenderContext::setActiveFrameIdx(int idx)
{
    activeFrameIndex = idx;
}

bool RenderContext::isPrepared() const
{
    return prepared;
}

void RenderContext::draw(const Scene& scene)
{
    auto& commandBuffer = commandBuffers[activeFrameIndex];
    auto& pipeline = device.getResourceCache().requestPipeline(pipelineState);

    commandBuffer.bindPipeline(pipeline.getHandle());
}

RenderGraph& RenderContext::getRenderGraph() const
{
    return *renderGraph;
}

PipelineState& RenderContext::getPipelineState()
{
    return pipelineState;
}

sg::SgImage& RenderContext::getCurHwtexture()
{
    return hwTextures[activeFrameIndex];
}

void RenderContext::bindBuffer(uint32_t setId, const Buffer& buffer, VkDeviceSize offset, VkDeviceSize range,
                               uint32_t binding, uint32_t array_element)
{
    resourceSets[setId].bindBuffer(buffer, offset, range, binding, array_element);
}

void RenderContext::bindImage(uint32_t setId, const ImageView& view, const Sampler& sampler, uint32_t binding,
                              uint32_t array_element)
{
    resourceSets[setId].bindImage(view, sampler, binding, array_element);
}

const std::unordered_map<uint32_t, ResourceSet>& RenderContext::getResourceSets() const
{
    return resourceSets;
}

void RenderContext::flushDescriptorState(CommandBuffer& commandBuffer, VkPipelineBindPoint pipeline_bind_point)
{
    auto& pipelineLayout = pipelineState.getPipelineLayout();
    for (auto& resourceSetIt : resourceSets)
    {
        BindingMap<VkDescriptorBufferInfo> buffer_infos;
        BindingMap<VkDescriptorImageInfo> image_infos;

        std::vector<uint32_t> dynamic_offsets;

        auto descriptorSetID = resourceSetIt.first;
        auto& resourceSet = resourceSetIt.second;

        if (!pipelineState.getPipelineLayout().hasLayout(descriptorSetID))
            continue;

        auto& descriptorSetLayout = pipelineLayout.getDescriptorLayout(descriptorSetID);

        for (auto& bindingIt : resourceSet.getResourceBindings())
        {
            auto bindingIndex = bindingIt.first;
            auto& bindingResources = bindingIt.second;

            for (auto& elementIt : bindingResources)
            {
                auto arrayElement = elementIt.first;
                auto& resourceInfo = elementIt.second;

                auto& buffer = resourceInfo.buffer;
                auto& sampler = resourceInfo.sampler;
                auto& imageView = resourceInfo.image_view;

                auto& bindingInfo = descriptorSetLayout.getLayoutBindingInfo(bindingIndex);

                if (buffer != nullptr)
                {
                    VkDescriptorBufferInfo bufferInfo{
                        .buffer = buffer->getHandle(), .offset = resourceInfo.offset, .range = resourceInfo.range
                    };

                    buffer_infos[bindingIndex][arrayElement] = bufferInfo;
                }

                if (imageView != nullptr && sampler != nullptr)
                {
                    VkDescriptorImageInfo imageInfo{
                        .sampler = sampler->getHandle(),
                        .imageView = imageView->getHandle(),
                        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    };

                    image_infos[bindingIndex][arrayElement] = imageInfo;
                }
            }
        }

        auto descriptorPool = device.getResourceCache().requestDescriptorPool(descriptorSetLayout);
        auto descriptorSet = device.getResourceCache().requestDescriptorSet(
            descriptorSetLayout, descriptorPool, buffer_infos, image_infos);

        commandBuffer.bindDescriptorSets(pipeline_bind_point, pipelineLayout.getHandle(), descriptorSetID,
                                         {descriptorSet}, dynamic_offsets);
    }
}
