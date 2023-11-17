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

#include "VertexData.h"
#include "Common/ResourceCache.h"
#include "Common/VkCommon.h"
#include "Images/Sampler.h"

RenderContext* RenderContext::g_context = nullptr;

void FrameResource::reset()
{
    for (auto& it : bufferPools)
        it.second->reset();
}

FrameResource::FrameResource(Device& device)
{
    for (auto& it : supported_usage_map)
    {
        bufferPools.emplace(
            it.first,
            std::move(std::make_unique<BufferPool>(device, BUFFER_POOL_BLOCK_SIZE * it.second * 1024, it.first)));
    }
}


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


    VkCommandBufferAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = device.getCommandPool().getHandle();
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = getSwapChainImageCount();
    std::vector<VkCommandBuffer> vkCommandBuffers(getSwapChainImageCount());

    VK_CHECK_RESULT(vkAllocateCommandBuffers(device.getHandle(), &allocateInfo, vkCommandBuffers.data()))

    for (uint32_t i = 0; i < getSwapChainImageCount(); i++)
    {
        frameResources.emplace_back(std::make_unique<FrameResource>(device));
        frameResources.back()->commandBuffer = std::make_unique<CommandBuffer>(vkCommandBuffers[i]);
    }
}

CommandBuffer& RenderContext::begin()
{
    assert(prepared && "RenderContext not prepared for rendering, call prepare()");

    if (!frameActive)
    {
        beginFrame();
    }

    frameResources[activeFrameIndex].reset();

    if (acquiredSem == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Couldn't begin frame");
    }
    auto& queue = device.getQueueByFlag(VK_QUEUE_GRAPHICS_BIT, 0);


    // return getActiveRenderFrame().requestCommandBuffer(queue);
}

CommandBuffer& RenderContext::beginFrame()
{
    // assert(activeFrameIndex < frames.size());
    if (swapchain)
    {
        VK_CHECK_RESULT(swapchain->acquireNextImage(activeFrameIndex, semaphores.presentFinishedSem, VK_NULL_HANDLE));
    }
    frameActive = true;

    frameResources[activeFrameIndex]->reset();
    clearResourceSets();

    auto& commandBuffer = getCommandBuffer();
    commandBuffer.beginRecord(0);


    commandBuffer.setViewport(0, {
                                  vkCommon::initializers::viewport(float(getSwapChainExtent().width),
                                                                   float(getSwapChainExtent().height), 0.0f, 1.0f)
                              });
    commandBuffer.setScissor(0, {
                                 vkCommon::initializers::rect2D(float(getSwapChainExtent().width),
                                                                float(getSwapChainExtent().height), 0, 0)
                             });

    return commandBuffer;
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
    // ImageMemoryBarrier memory_barrier{};
    // memory_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    // memory_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    // memory_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    // memory_barrier.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    // memory_barrier.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    // buffer.imageMemoryBarrier(
    //     getCurHwtexture().getVkImageView(),
    //     memory_barrier);


    getCurHwtexture().getVkImage().transitionLayout(buffer, VulkanLayout::PRESENT,
                                                    getCurHwtexture().getVkImageView().getSubResourceRange());
    buffer.endRecord();


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
        VK_CHECK_RESULT(queue.present(present_info))
    }
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

SgImage& RenderContext::getCurHwtexture()
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

void RenderContext::bindInput(uint32_t setId, const ImageView& view, uint32_t binding, uint32_t array_element)
{
    resourceSets[setId].bindInput(view, binding, array_element);
}

void RenderContext::bindMaterial(const gltfLoading::Material& material)
{
    auto& descriptorLayout = pipelineState.getPipelineLayout().getDescriptorLayout(0);

    for (auto& texture : material.textures)
    {
        if (descriptorLayout.hasLayoutBinding(texture.first))
        {
            auto& binding = descriptorLayout.getLayoutBindingInfo(texture.first);
            bindImage(0, texture.second->image->getVkImageView(), texture.second->getSampler(), binding.binding, 0);
        }
    }
}

void RenderContext::bindPrimitive(const gltfLoading::Primitive& primitive)
{
    VertexInputState vertexInputState;

    for (const auto& inputResource : pipelineState.getPipelineLayout().getShaderResources(ShaderResourceType::Input,
             VK_SHADER_STAGE_VERTEX_BIT))
    {
        gltfLoading::VertexAttribute attribute{};
        if (!primitive.getVertexAttribute(inputResource.name, attribute))
        {
            continue;
        }
        VkVertexInputAttributeDescription vertex_attribute{};
        vertex_attribute.binding = inputResource.location;
        vertex_attribute.format = attribute.format;
        vertex_attribute.location = inputResource.location;
        vertex_attribute.offset = attribute.offset;

        vertexInputState.attributes.push_back(vertex_attribute);

        VkVertexInputBindingDescription vertex_binding{};
        vertex_binding.binding = inputResource.location;
        vertex_binding.stride = attribute.stride;

        vertexInputState.bindings.push_back(vertex_binding);

        if (primitive.vertexBuffers.contains(inputResource.name))
        {
            std::vector<const Buffer*> buffers = {&primitive.getVertexBuffer(inputResource.name)};
            getCommandBuffer().bindVertexBuffer(inputResource.location, buffers, {0});
            getCommandBuffer().bindIndicesBuffer(*primitive.indexBuffer,0);
        }
    }

    pipelineState.setVertexInputState(vertexInputState);
}


// const std::unordered_map<uint32_t, ResourceSet>& RenderContext::getResourceSets() const
// {
//     return resourceSets;
// }

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
        BindingMap<VkDescriptorImageInfo> imageInfos;

        std::vector<uint32_t> dynamic_offsets;

        auto descriptorSetID = resourceSetIt.first;
        auto& resourceSet = resourceSetIt.second;

        if (!resourceSet.isDirty())
            continue;

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

                if (imageView != nullptr || sampler != nullptr)
                {
                    VkDescriptorImageInfo imageInfo{

                        .imageView = imageView->getHandle(),
                        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    };

                    if (sampler)
                        imageInfo.sampler = sampler->getHandle();

                    if (imageView != nullptr)
                    {
                        // Add image layout info based on descriptor type
                        switch (bindingInfo.descriptorType)
                        {
                        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                            break;
                        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                            if (isDepthOrStencilFormat(imageView->getFormat()))
                            {
                                imageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
                            }
                            else
                            {
                                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                            }
                            break;
                        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                            imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                            break;

                        default:
                            continue;
                        }
                    }

                    imageInfos[bindingIndex][arrayElement] = imageInfo;
                }
            }
        }

        auto descriptorPool = device.getResourceCache().requestDescriptorPool(descriptorSetLayout);
        auto descriptorSet = device.getResourceCache().requestDescriptorSet(
            descriptorSetLayout, descriptorPool, buffer_infos, imageInfos);

        commandBuffer.bindDescriptorSets(pipeline_bind_point, pipelineLayout.getHandle(), descriptorSetID,
                                         {descriptorSet}, dynamic_offsets);
    }
}


void RenderContext::flushPipelineState(CommandBuffer& commandBuffer)
{
    commandBuffer.bindPipeline(device.getResourceCache().requestPipeline(this->getPipelineState()));
}

void RenderContext::bindPipelineLayout(PipelineLayout& layout)
{
    pipelineState.setPipelineLayout(layout);
}

void RenderContext::draw(CommandBuffer& commandBuffer, gltfLoading::Model& model)
{
    model.bindBuffer(commandBuffer);
    for (auto& node : model.linearNodes)
        draw(commandBuffer, *node);

    // VkDeviceSize offsets[1] = {0};
    // VkBuffer vertexBuffer[] = {vertices->getHandle()};
    // vkCmdBindVertexBuffers(commandBuffer.getHandle(), 0, 1, vertexBuffer, offsets);
    // vkCmdBindIndexBuffer(commandBuffer.getHandle(), indices->getHandle(), 0, VK_INDEX_TYPE_UINT32);
    // for (auto &node: linearNodes)
    //     draw(*node, commandBuffer, renderFlags, pipelineLayout, bindImageSet);
}

void RenderContext::flushAndDrawIndexed(CommandBuffer& commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                                        uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{
    flush(commandBuffer);
    commandBuffer.drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void RenderContext::flushAndDraw(CommandBuffer& commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
                                 uint32_t firstVertex, uint32_t firstInstance)
{
    flush(commandBuffer);
    commandBuffer.draw(vertexCount, instanceCount, firstVertex, firstInstance);
}


void RenderContext::drawLightingPass(CommandBuffer& commandBuffer)
{
    struct Poses
    {
        glm::vec3 lightPos, cameraPos;
    };

    auto buffer = allocateBuffer(sizeof(Poses), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    Poses poses{.lightPos = {0.0f, 2.5f, 0.0f}, .cameraPos = camera->position};
    buffer.buffer->uploadData(&poses, buffer.size, buffer.offset);
    bindBuffer(0, *buffer.buffer, buffer.offset, buffer.size, 3, 0);

    flushPipelineState(commandBuffer);
    flushDescriptorState(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
    commandBuffer.draw(3, 1, 0, 0);
}

void RenderContext::beginRenderPass(CommandBuffer& commandBuffer, RenderTarget& renderTarget,
                                    const std::vector<SubpassInfo>& subpassInfos)
{
    // auto hwTextures = renderTarget.getHwTextures();
    //
    // //转换图像布局
    // {
    //     // Image 0 is the swapchain
    //     ImageMemoryBarrier memoryBarrier{};
    //     memoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    //     memoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    //     memoryBarrier.srcAccessMask = 0;
    //     memoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    //     memoryBarrier.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //     memoryBarrier.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //
    //     ImageMemoryBarrier depthMemoryBarrier{};
    //     depthMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    //     depthMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    //     depthMemoryBarrier.srcAccessMask = 0;
    //     depthMemoryBarrier.dstAccessMask =
    //         VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    //     depthMemoryBarrier.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    //     depthMemoryBarrier.dstStageMask =
    //         VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    //
    //
    //     // commandBuffer.imageMemoryBarrier(hwTextures[0]->getVkImageView(), memoryBarrier);
    //
    //     // Skip 1 as it is handled later as a depth-stencil attachment
    //     for (size_t i = 0; i < renderTarget.getHwTextures().size(); ++i)
    //     {
    //         if (isDepthOrStencilFormat(hwTextures[i]->getFormat()))
    //         {
    //             commandBuffer.imageMemoryBarrier(hwTextures[i]->getVkImageView(), depthMemoryBarrier);
    //         }
    //         else
    //         {
    //             if (i > 1)
    //             {
    //                 memoryBarrier.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    //                 memoryBarrier.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
    //             }
    //             commandBuffer.imageMemoryBarrier(hwTextures[i]->getVkImageView(), memoryBarrier);
    //         }
    //     }
    // }

    auto& renderPass = device.getResourceCache().requestRenderPass(renderTarget.getAttachments(), subpassInfos);
    auto& framebuffer = device.getResourceCache().requestFrameBuffer(
        renderTarget, renderPass);
    commandBuffer.beginRenderPass(renderPass, framebuffer, renderTarget.getDefaultClearValues(), {});

    ColorBlendState colorBlendState = pipelineState.getColorBlendState();
    colorBlendState.attachments.resize(renderPass.getColorOutputCount(0));
    pipelineState.setColorBlendState(colorBlendState);

    pipelineState.setRenderPass(renderPass);
    pipelineState.setSubpassIndex(0);
}

void RenderContext::nextSubpass(CommandBuffer& commandBuffer)
{
    clearResourceSets();

    uint32_t subpassIdx = pipelineState.getSubpassIndex();
    pipelineState.setSubpassIndex(subpassIdx + 1);

    ColorBlendState colorBlendState = pipelineState.getColorBlendState();
    colorBlendState.attachments.resize(pipelineState.getRenderPass()->getColorOutputCount(subpassIdx + 1));
    pipelineState.setColorBlendState(colorBlendState);

    vkCmdNextSubpass(commandBuffer.getHandle(), {});
}

void RenderContext::flush(CommandBuffer& commandBuffer)
{
    flushPipelineState(commandBuffer);
    flushDescriptorState(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}

void RenderContext::endRenderPass(CommandBuffer& commandBuffer)
{
    commandBuffer.endPass();
    resourceSets.clear();
    pipelineState.reset();
}

void RenderContext::clearResourceSets()
{
    resourceSets.clear();
}


void RenderContext::draw(CommandBuffer& commandBuffer, gltfLoading::Node& node)
{
    auto& pipelineLayout = pipelineState.getPipelineLayout();
    //fixme 
    VertexInputState vertexInputState{};
    vertexInputState.bindings = {Vertex::getBindingDescription()};
    vertexInputState.attributes = Vertex::getAttributeDescriptions();


    pipelineState.setVertexInputState(vertexInputState);

    flushPipelineState(commandBuffer);


    auto allocation = allocateBuffer(sizeof(GlobalUniform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    //todo: use camera data here
    GlobalUniform uniform{
        .model = node.getMatrix(), .view = camera->matrices.view, .proj = camera->matrices.perspective
    };
    allocation.buffer->uploadData(&uniform, allocation.size, allocation.offset);
    bindBuffer(0, *allocation.buffer, allocation.offset, allocation.size, 0, 0);

    auto& descriptorLayout = pipelineLayout.getDescriptorLayout(0);

    for (auto& prim : node.mesh->primitives)
    {
        const auto& material = prim->material;
        for (auto& texture : material.textures)
        {
            if (descriptorLayout.hasLayoutBinding(texture.first))
            {
                auto& binding = descriptorLayout.getLayoutBindingInfo(texture.first);
                bindImage(0, texture.second->image->getVkImageView(), texture.second->getSampler(), binding.binding, 0);
            }
        }
        flushDescriptorState(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
        commandBuffer.drawIndexed(prim->indexCount, 1, prim->firstIndex, 0, 0);
    }
    for (auto& child : node.children)
        draw(commandBuffer, node);
}

BufferAllocation RenderContext::allocateBuffer(VkDeviceSize allocateSize, VkBufferUsageFlags usage)
{
    auto& frameResource = frameResources[activeFrameIndex];
    // assert(frameResource.bufferPools.contains(usage), "Buffer usage not contained");
    return frameResource->bufferPools.at(usage)->AllocateBufferBlock(allocateSize);
}

CommandBuffer& RenderContext::getCommandBuffer()
{
    return *frameResources[activeFrameIndex]->commandBuffer;
}
