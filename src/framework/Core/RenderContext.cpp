//
// Created by 打工人 on 2023/3/30.
//
#include "RenderContext.h"
#include "Core/CommandBuffer.h"
#include "Core/Device/Device.h"
#include "SwapChain.h"
#include "Queue.h"
#include "Core/Images/Image.h"
#include "RenderTarget.h"
#include "Core/FrameBuffer.h"
#include "Core/Buffer.h"

#include "PlatForm/Window.h"
#include "Common/ResourceCache.h"
#include "Common/VkCommon.h"
#include "Images/Sampler.h"
#include "RayTracing/Accel.h"
#include "RayTracing/SbtWarpper.h"

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
    swapchain = std::make_unique<SwapChain>(device, surface, window.getExtent());
    if (swapchain)
    {
        surfaceExtent = swapchain->getExtent();

        VkExtent3D extent{surfaceExtent.width, surfaceExtent.height, 1};

        for (auto& image_handle : swapchain->getImages())
        {
            hwTextures.emplace_back(device, image_handle, extent, swapchain->getImageFormat(), swapchain->getUseage());
        }
    }

    VkSemaphoreCreateInfo semaphoreCreateInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VK_CHECK_RESULT(
        vkCreateSemaphore(device.getHandle(), &semaphoreCreateInfo, nullptr, &semaphores.renderFinishedSem));
    VK_CHECK_RESULT(
        vkCreateSemaphore(device.getHandle(), &semaphoreCreateInfo, nullptr, &semaphores.presentFinishedSem));


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


CommandBuffer& RenderContext::beginFrame()
{
    // assert(activeFrameIndex < frames.size());
    if (swapchain)
    {
        VK_CHECK_RESULT(swapchain->acquireNextImage(activeFrameIndex, semaphores.presentFinishedSem, VK_NULL_HANDLE));
    }
    frameActive = true;

    frameResources[activeFrameIndex]->reset();
    clearPassResources();

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
}

uint32_t RenderContext::getActiveFrameIndex() const
{
    return activeFrameIndex;
}

void RenderContext::submitAndPresent(CommandBuffer& buffer, VkFence fence)
{
    getCurHwtexture().getVkImage().transitionLayout(buffer, VulkanLayout::PRESENT,
                                                    getCurHwtexture().getVkImageView().getSubResourceRange());
    buffer.endRecord();


    auto queue = device.getQueueByFlag(VK_QUEUE_GRAPHICS_BIT, 0);

    VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &semaphores.presentFinishedSem;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &semaphores.renderFinishedSem;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = buffer.getHandlePointer();

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


        if (queue.present(present_info) == VK_ERROR_OUT_OF_DATE_KHR)
        {
            handleSurfaceChanges();
        }

        frameActive = false;
    }

    queue.wait();
}

void RenderContext::submit(CommandBuffer& commandBuffer, bool waiteFence)
{
    commandBuffer.endRecord();
    auto queue = device.getQueueByFlag(VK_QUEUE_GRAPHICS_BIT, 0);

    VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = commandBuffer.getHandlePointer();

    VkFence fence = VK_NULL_HANDLE;
    if(waiteFence)
    {
        VkFenceCreateInfo fenceInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        vkCreateFence(device.getHandle(), &fenceInfo, nullptr, &fence);
    }

    queue.submit({submitInfo}, fence);
    queue.wait();
    if(waiteFence)
    {
        vkWaitForFences(device.getHandle(),1, &fence, VK_TRUE, UINT64_MAX);
    }
}

VkFormat RenderContext::getSwapChainFormat() const
{
    return swapchain->getImageFormat();
}

VkExtent2D RenderContext::getSwapChainExtent() const
{
    return swapchain->getExtent();
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


uint32_t RenderContext::getSwapChainImageCount() const
{
    return swapchain->getImageCount();
}


void RenderContext::setActiveFrameIdx(int idx)
{
    activeFrameIndex = idx;
}

bool RenderContext::isPrepared() const
{
    return prepared;
}

PipelineState& RenderContext::getPipelineState()
{
    return pipelineState;
}

SgImage& RenderContext::getCurHwtexture()
{
    return hwTextures[activeFrameIndex];
}

// RenderContext & RenderContext::bindBuffer(uint32_t setId, const Buffer& buffer, VkDeviceSize offset, VkDeviceSize range,
//                                uint32_t binding, uint32_t array_element)
// {
//     resourceSets[setId].bindBuffer(buffer, offset, range, binding, array_element);
//     return *this;
// }

RenderContext &  RenderContext::bindBuffer(uint32_t binding, const Buffer& buffer, VkDeviceSize offset, VkDeviceSize range,
    uint32_t setId, uint32_t array_element)
{
    if(range == 0) range = buffer.getSize();
    resourceSets[setId].bindBuffer(buffer, offset, range, binding, array_element);
    return *this;
}

RenderContext& RenderContext::bindAcceleration(uint32_t setId, const Accel& acceleration, uint32_t binding,
                                               uint32_t array_element)
{
    resourceSets[setId].bindAccel1(acceleration,binding,array_element);
    return *this;
}

RenderContext & RenderContext::bindImage(uint32_t setId, const ImageView& view, const Sampler& sampler, uint32_t binding,
                                         uint32_t array_element)
{
    resourceSets[setId].bindImage(view, sampler, binding, array_element);
    return *this;
}

RenderContext & RenderContext::bindInput(uint32_t setId, const ImageView& view, uint32_t binding, uint32_t array_element)
{
    resourceSets[setId].bindInput(view, binding, array_element);
    return *this;

}

RenderContext & RenderContext::bindMaterial(const Material& material)
{
    auto& descriptorLayout = pipelineState.getPipelineLayout().getDescriptorLayout(0);

    for (auto& texture : material.textures)
    {
        if (descriptorLayout.hasLayoutBinding(texture.first))
        {
            auto& binding = descriptorLayout.getLayoutBindingInfo(texture.first);
            bindImage(0, texture.second.image->getVkImageView(), texture.second.getSampler(), binding.binding, 0);
        }
    }
    return *this;

}

RenderContext & RenderContext::bindPrimitive(const Primitive& primitive)
{
    VertexInputState vertexInputState;

    for (const auto& inputResource : pipelineState.getPipelineLayout().getShaderResources(ShaderResourceType::Input,
             VK_SHADER_STAGE_VERTEX_BIT))
    {
        VertexAttribute attribute{};
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
        }
    }
    getCommandBuffer().bindIndicesBuffer(*primitive.indexBuffer, primitive.indexType);

    

    pipelineState.setVertexInputState(vertexInputState);

    return bindMaterial(primitive.material);
}


const std::unordered_map<uint32_t, ResourceSet>& RenderContext::getResourceSets() const
{
    return resourceSets;
}

VkPipelineBindPoint RenderContext::getPipelineBindPoint() const
{
    switch (pipelineState.getPipelineType())
    {
        case PIPELINE_TYPE::E_GRAPHICS:
            return VK_PIPELINE_BIND_POINT_GRAPHICS;
        case PIPELINE_TYPE::E_COMPUTE:
            return VK_PIPELINE_BIND_POINT_COMPUTE;
        case PIPELINE_TYPE::E_RAY_TRACING:
            return VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
        default:
            LOGE("pipeline bind point not support");
    }
    return VK_PIPELINE_BIND_POINT_GRAPHICS;
}


void RenderContext::flushDescriptorState(CommandBuffer& commandBuffer, VkPipelineBindPoint pipeline_bind_point)
{
    auto& pipelineLayout = pipelineState.getPipelineLayout();
    for (auto& resourceSetIt : resourceSets)
    {
        //setId -> pair(binding,arrayElement)
        BindingMap<VkDescriptorBufferInfo> bufferInfos;
        BindingMap<VkDescriptorImageInfo> imageInfos;
        BindingMap<VkWriteDescriptorSetAccelerationStructureKHR> accelerationInfos;

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
                auto& accel = resourceInfo.accel;

                if (!descriptorSetLayout.hasLayoutBinding(bindingIndex))
                    continue;
                auto& bindingInfo = descriptorSetLayout.getLayoutBindingInfo(bindingIndex);

                if (buffer != nullptr)
                {
                    VkDescriptorBufferInfo bufferInfo{
                        .buffer = buffer->getHandle(), .offset = resourceInfo.offset, .range = resourceInfo.range
                    };

                    bufferInfos[bindingIndex][arrayElement] = bufferInfo;
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
                    // if( resourceInfo.layout != VK_IMAGE_LAYOUT_UNDEFINED)
                 // imageInfo.imageLayout =  ImageUtil::getVkImageLayout(imageView->getImage().getLayout());
                    imageInfos[bindingIndex][arrayElement] = imageInfo;
                }

                if(accel!=nullptr)
                {
                    VkWriteDescriptorSetAccelerationStructureKHR accelInfo{
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR,
                        .accelerationStructureCount = 1,
                        .pAccelerationStructures = &accel->accel
                    };
                    accelerationInfos[bindingIndex][arrayElement] = accelInfo;
                }
            }


        }        auto descriptorPool = device.getResourceCache().requestDescriptorPool(descriptorSetLayout);
        auto descriptorSet = device.getResourceCache().requestDescriptorSet(
            descriptorSetLayout, descriptorPool, bufferInfos, imageInfos,accelerationInfos);

        commandBuffer.bindDescriptorSets(pipeline_bind_point, pipelineLayout.getHandle(), descriptorSetID,
                                         {descriptorSet}, dynamic_offsets);
    }
}


void RenderContext::flushPipelineState(CommandBuffer& commandBuffer)
{
    commandBuffer.bindPipeline(device.getResourceCache().requestPipeline(this->getPipelineState()),
                               getPipelineBindPoint());
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

void RenderContext::traceRay(CommandBuffer& commandBuffer,VkExtent3D dims)
{
    CHECK_RESULT((getPipelineState().getPipelineType() == PIPELINE_TYPE::E_RAY_TRACING));
    if(dims.width == 0 || dims.height == 0 || dims.depth == 0)
    {
        dims = {getSwapChainExtent().width,getSwapChainExtent().height,1};
    }
    flush(commandBuffer);
    auto & pipeline = device.getResourceCache().requestPipeline(this->getPipelineState());
    auto & sbtWarpper = pipeline.getSbtWarpper();
    auto & shaderBindingTable = sbtWarpper.getRegions();
    vkCmdTraceRaysKHR(commandBuffer.getHandle(), &shaderBindingTable[0], &shaderBindingTable[1],
                      &shaderBindingTable[2], &shaderBindingTable[3], dims.width, dims.height, dims.depth);
}

void RenderContext::dispath(CommandBuffer& commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
    uint32_t groupCountZ)
{
    flush(commandBuffer);
    vkCmdDispatch(commandBuffer.getHandle(), groupCountX, groupCountY, groupCountZ);
}


void RenderContext::beginRenderPass(CommandBuffer& commandBuffer, RenderTarget& renderTarget,
                                    const std::vector<SubpassInfo>& subpassInfos)
{
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
    clearPassResources();

    uint32_t subpassIdx = pipelineState.getSubpassIndex();
    pipelineState.setSubpassIndex(subpassIdx + 1);

    ColorBlendState colorBlendState = pipelineState.getColorBlendState();
    colorBlendState.attachments.resize(pipelineState.getRenderPass()->getColorOutputCount(subpassIdx + 1));
    pipelineState.setColorBlendState(colorBlendState);

    vkCmdNextSubpass(commandBuffer.getHandle(), {});

    
}

void RenderContext::flushPushConstantStage(CommandBuffer& commandBuffer)
{
    // commandBuffer
    if (storePushConstants.empty())
        return;


    auto& pipelineLayout = pipelineState.getPipelineLayout();
    auto pushConstantRange = pipelineLayout.getPushConstantRangeStage(storePushConstants.size());

    vkCmdPushConstants(commandBuffer.getHandle(), pipelineLayout.getHandle(), pushConstantRange,
                       0, storePushConstants.size(), storePushConstants.data());
    storePushConstants.clear();
}


void RenderContext::flush(CommandBuffer& commandBuffer)
{
    flushPipelineState(commandBuffer);
    flushDescriptorState(commandBuffer, getPipelineBindPoint());
    flushPushConstantStage(commandBuffer);
}

void RenderContext::endRenderPass(CommandBuffer& commandBuffer,RenderTarget& renderTarget)
{
    commandBuffer.endPass();
    resourceSets.clear();

    auto & finalLayouts = pipelineState.getRenderPass()->getAttachmentFinalLayouts();
    for(uint32_t i = 0; i<finalLayouts.size();i++ )
    {
        renderTarget.getImage(i).setLayout(finalLayouts[i]);
    }
    
    pipelineState.reset();
}

void RenderContext::clearPassResources()
{
    resourceSets.clear();
    storePushConstants.clear();
}

RenderContext & RenderContext::bindPushConstants(std::vector<uint8_t>& pushConstants)
{
    auto size = pushConstants.size() + storePushConstants.size();
    if (size > maxPushConstantSize)
    {
        LOGE("Push Constant Size is too large,device support {},but current size is {}", maxPushConstantSize, size);
    }
    storePushConstants.insert(storePushConstants.end(), pushConstants.begin(), pushConstants.end());
    return *this;
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

void RenderContext::handleSurfaceChanges()
{
    VkSurfaceCapabilitiesKHR surface_properties;
    VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.getPhysicalDevice(),
        swapchain->getSurface(),
        &surface_properties))

    if (surfaceExtent.width != surface_properties.currentExtent.width || surfaceExtent.height != surface_properties.
        currentExtent.height)
    {
        device.waitIdle();
        surfaceExtent = surface_properties.currentExtent;
        recrateSwapChain(surfaceExtent);
    }
}

void RenderContext::recrateSwapChain(VkExtent2D extent)
{
    device.getResourceCache().clearFrameBuffers();

    hwTextures.clear();

    auto surface = swapchain->getSurface();
    swapchain = nullptr;
    swapchain = std::make_unique<SwapChain>(device, surface, extent);

    if (swapchain)
    {
        surfaceExtent = swapchain->getExtent();

        VkExtent3D extent3d{surfaceExtent.width, surfaceExtent.height, 1};

        for (auto& image_handle : swapchain->getImages())
        {
            auto swapChainImage = Image(device, image_handle,
                                        extent3d,
                                        swapchain->getImageFormat(),
                                        swapchain->getUseage());
            hwTextures.emplace_back(device, image_handle, extent3d, swapchain->getImageFormat(),
                                    swapchain->getUseage());
        }
    }
}

void RenderContext::copyBuffer(Buffer& src, Buffer& dst)
{
    auto commandBuffer    = device.createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY,true);
    VkBufferCopy    copy_region  = {};
    copy_region.size             = src.getSize();
    vkCmdCopyBuffer(commandBuffer.getHandle(), src.getHandle(),dst.getHandle(), 1, &copy_region);
    submit(commandBuffer);
}
