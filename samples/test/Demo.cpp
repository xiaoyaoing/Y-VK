//
// Created by 打工人 on 2023/3/19.
//

#include "Demo.h"
void Demo::initVk()
{
    Application::initVk();
    createGraphicsPipeline();
    createImages();
    createDescriptorSet();
    createUniformBuffers();
    createMeshes();
}

void Demo::drawFrame()
{
    vkWaitForFences(_device->getHandle(), 1, &inFlightFences[curFrameCount], VK_TRUE, UINT64_MAX);
    uint32_t imageIndex;
    auto result = vkAcquireNextImageKHR(_device->getHandle(), _swapChain->getHandle(), UINT64_MAX, imageAvailableSemaphores[curFrameCount],
                                        VK_NULL_HANDLE,
                                        &imageIndex);
    //    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    //        reCreateSwapChain();
    //        return;
    //    }

    updateUnifomBuffers();

    vkResetFences(_device->getHandle(), 1, &inFlightFences[curFrameCount]);

    auto commandBuffer = commandBuffers[curFrameCount];
    commandBuffer->beginRecord(1);
    std::vector<VkClearValue> clearValues(2);
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};
    commandBuffer->beginRenderPass(renderPass->getHandle(), swapChainFrameBuffers[curFrameCount]->getHandle(),
                                   clearValues, _swapChain->getExtent());

    for (const auto &mesh : meshes)
        mesh->bindOnly(commandBuffer->getHandle());
    commandBuffer->bindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0,
                                      {_descriptorSet[curFrameCount]}, {});
    commandBuffer->bindPipeline(graphicsPipeline->getHandle());
    for (const auto &mesh : meshes)
        mesh->drawOnly(commandBuffer->getHandle());
    commandBuffer->endRecord();

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[curFrameCount]};
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

    result = vkQueueSubmit(graphicsQueue->getHandle(), 1, &submitInfo, inFlightFences[curFrameCount]);
    if (result != VK_SUCCESS)
    {
        RUN_TIME_ERROR("failed to submit queue buffer");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapChains[] = {_swapChain->getHandle()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    // vkDeviceWaitIdle(device);
    // std::this_thread::sleep_for(std::chrono::milliseconds (10));

    result = vkQueuePresentKHR(presentQueue->getHandle(), &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || frameBufferResized)
    {
        frameBufferResized = false;
        reCreateSwapChain();
        return;
    }

    curFrameCount = (curFrameCount + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Demo::createGraphicsPipeline()
{
    PipelineInfo pipelineInfo;

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 0;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    VkDescriptorSetLayout pSetLayouts[] = {_descriptorLayout.getHandle()};
    pipelineLayoutCreateInfo.pSetLayouts = pSetLayouts;
    VkPipelineLayout pipelineLayout;
    if (vkCreatePipelineLayout(_device->getHandle(), &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS)

        RUN_TIME_ERROR("Failed to create pipeline layout");
    Pipeline p(pipelineInfo, _device, std::vector<VkDescriptorSetLayoutBinding>({Vertex::getBindingDescription()}), {Vertex::getAttributeDescriptions()}, pipelineLayout, _renderPass->getHandle());
    _pipeline = std::make_shared<>()
}

void Demo::createMeshes()
{
}

void Demo::createImages()
{
}

void Demo::createUniformBuffers()
{
}

void Demo::createDescriptorSet()
{
}

void Demo::updateUnifomBuffers()
{
}
